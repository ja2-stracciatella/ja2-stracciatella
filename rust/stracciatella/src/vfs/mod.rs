//! This module implements virtual filesystems.
//!
//! The paths are case insensitive.
//! It does not support path components `.` and `..`.
#![allow(dead_code)]

#[cfg(target_os = "android")]
pub mod android;
pub mod dir;
pub mod slf;
pub mod tmp;

use std::collections::BTreeSet;
use std::fmt;
use std::io;
use std::io::ErrorKind;
use std::path::{Path, PathBuf};
use std::sync::Arc;

use json_patch::Patch;
use log::{info, warn};
use serde_json::Value;

use crate::EngineOptions;
use crate::fs;
use crate::json;
use crate::mods::ModManager;
use crate::mods::ModPath;
use crate::unicode::Nfc;
use crate::vfs::dir::DirFs;
use crate::vfs::slf::SlfFs;

pub trait VfsFile:
    io::Read + io::Seek + io::Write + fmt::Debug + fmt::Display + Send + Sync
{
    /// Returns the length of the file
    fn len(&self) -> io::Result<u64>;

    /// Returns true if the virtual file is empty.
    fn is_empty(&self) -> io::Result<bool> {
        Ok(self.len()? == 0)
    }
}

/// Metadata about an entry in a VFS layer.
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct VfsMetadata {
    /// True if the entry is a directory.
    pub is_dir: bool,
    /// Length of the entry in bytes. Zero for directories.
    pub len: u64,
    /// Last modification time in seconds since the unix epoch, if the layer can report one.
    pub modified_secs: Option<f64>,
    /// True if the entry itself cannot be modified.
    pub read_only: bool,
}

/// The options used to open a file in a VFS layer.
///
/// Mirrors the subset of [`std::fs::OpenOptions`] that the engine uses.
#[derive(Debug, Clone, Copy, Default, PartialEq, Eq)]
pub struct VfsOpenOptions {
    pub read: bool,
    pub write: bool,
    pub append: bool,
    pub truncate: bool,
    pub create: bool,
    pub create_new: bool,
}

impl VfsOpenOptions {
    /// True if the options would modify the file in any way.
    pub fn is_write(&self) -> bool {
        self.write || self.append || self.truncate || self.create || self.create_new
    }
}

/// Builds the error that the write operations of a read-only layer return.
///
/// This is `Unsupported` and not `PermissionDenied` on purpose, so that "this layer never accepts
/// writes" stays distinguishable from "the operating system refused this particular write".
pub fn read_only_error<T: fmt::Display + ?Sized>(layer: &T) -> io::Error {
    io::Error::new(
        io::ErrorKind::Unsupported,
        format!("{} is read-only", layer),
    )
}

pub trait VfsLayer: fmt::Debug + fmt::Display + Send + Sync {
    /// Opens a file in the VFS Layer
    fn open(&self, file_path: &Nfc) -> io::Result<Box<dyn VfsFile>>;
    /// Checks if a file exists in the VFS Layer
    fn exists(&self, file_path: &Nfc) -> io::Result<bool>;
    // Lists a directory in the VFS Layer
    fn read_dir(&self, file_path: &Nfc) -> io::Result<BTreeSet<Nfc>>;
    /// Lists files with a specific extension in a directory in the VFS Layer
    ///
    /// The extension has to be specified without a dot (e.g. "slf")
    fn read_dir_with_extension(
        &self,
        file_path: &Nfc,
        extension: &Nfc,
    ) -> io::Result<BTreeSet<Nfc>> {
        let extension = Nfc::caseless(&format!(".{}", extension));
        Ok(self
            .read_dir(file_path)?
            .into_iter()
            .filter(|path| path.ends_with(extension.as_str()))
            .collect())
    }

    /// True if this layer accepts writes.
    ///
    /// This expresses intent, not capability. A layer backed by a writable directory still reports
    /// false when it was mounted read-only, which is what keeps the engine from writing into the
    /// vanilla game data or into a mod directory.
    fn is_writable(&self) -> bool {
        false
    }

    /// Returns metadata for a path in the VFS layer.
    fn metadata(&self, file_path: &Nfc) -> io::Result<VfsMetadata> {
        let _ = file_path;
        Err(io::Error::new(
            io::ErrorKind::Unsupported,
            format!("{} does not support metadata", self),
        ))
    }

    /// Resolves the existing components of a path case insensitively.
    ///
    /// Layers that cannot resolve a path return it unchanged.
    fn resolve_existing_components(&self, file_path: &Nfc) -> Nfc {
        file_path.to_owned()
    }

    /// Opens a file with the given options, creating it when the options ask for it.
    ///
    /// Read-only layers only support options that do not modify the file.
    fn open_with_options(
        &self,
        file_path: &Nfc,
        options: VfsOpenOptions,
    ) -> io::Result<Box<dyn VfsFile>> {
        if !options.is_write() {
            return self.open(file_path);
        }
        Err(read_only_error(self))
    }

    /// Removes a file from the VFS layer.
    fn remove_file(&self, file_path: &Nfc) -> io::Result<()> {
        let _ = file_path;
        Err(read_only_error(self))
    }

    /// Creates a directory in the VFS layer. Does nothing when it already exists.
    fn create_dir(&self, file_path: &Nfc) -> io::Result<()> {
        let _ = file_path;
        Err(read_only_error(self))
    }

    /// Renames or moves a path within the VFS layer.
    fn rename(&self, from: &Nfc, to: &Nfc) -> io::Result<()> {
        let _ = (from, to);
        Err(read_only_error(self))
    }

    /// The amount of free space in bytes available to this layer.
    fn free_space(&self) -> io::Result<u64> {
        Err(io::Error::new(
            io::ErrorKind::Unsupported,
            format!("{} does not report free space", self),
        ))
    }

    /// Lists all files in a directory.
    ///
    /// The returned paths are relative to `dir_path`, so they are plain names unless `recursive`
    /// is set. They are sorted, because [`VfsLayer::read_dir`] returns a sorted set, and their
    /// case is folded like everything the VFS returns. Use
    /// [`VfsLayer::resolve_existing_components`] to get a name back as it is in the storage.
    fn find_all_files_in_dir(&self, dir_path: &Nfc, recursive: bool) -> io::Result<Vec<Nfc>> {
        let mut result = Vec::new();
        collect_dir_entries(
            self,
            dir_path,
            recursive,
            false,
            &Nfc::caseless(""),
            &mut result,
        )?;
        Ok(result)
    }

    /// Lists all directories in a directory.
    ///
    /// The returned paths are relative to `dir_path` and case folded, like
    /// [`VfsLayer::find_all_files_in_dir`].
    fn find_all_dirs_in_dir(&self, dir_path: &Nfc, recursive: bool) -> io::Result<Vec<Nfc>> {
        let mut result = Vec::new();
        collect_dir_entries(
            self,
            dir_path,
            recursive,
            true,
            &Nfc::caseless(""),
            &mut result,
        )?;
        Ok(result)
    }

    /// Removes all files in a directory, leaving subdirectories alone.
    fn erase_dir(&self, dir_path: &Nfc) -> io::Result<()> {
        for name in self.find_all_files_in_dir(dir_path, false)? {
            self.remove_file(&join_vfs_path(dir_path, &name))?;
        }
        Ok(())
    }
}

/// Joins two VFS paths, avoiding a leading or doubled separator.
pub fn join_vfs_path(base: &Nfc, path: &Nfc) -> Nfc {
    let base = base.trim_end_matches('/');
    if base.is_empty() {
        return path.to_owned();
    }
    Nfc::caseless_path(&format!("{}/{}", base, path))
}

/// Shared implementation of [`VfsLayer::find_all_files_in_dir`] and
/// [`VfsLayer::find_all_dirs_in_dir`].
///
/// `prefix` is the path of `dir_path` relative to the directory the caller asked about, so that the
/// recursive case reports `subdir/name` instead of a bare name.
fn collect_dir_entries<L: VfsLayer + ?Sized>(
    layer: &L,
    dir_path: &Nfc,
    recursive: bool,
    want_dirs: bool,
    prefix: &Nfc,
    result: &mut Vec<Nfc>,
) -> io::Result<()> {
    for name in layer.read_dir(dir_path)? {
        let full_path = join_vfs_path(dir_path, &name);
        let relative_path = join_vfs_path(prefix, &name);
        let is_dir = layer.metadata(&full_path)?.is_dir;
        if is_dir == want_dirs {
            result.push(relative_path.clone());
        }
        if is_dir && recursive {
            collect_dir_entries(
                layer,
                &full_path,
                recursive,
                want_dirs,
                &relative_path,
                result,
            )?;
        }
    }
    Ok(())
}

/// A virtual filesystem that mounts other filesystems.
#[derive(Debug, Default)]
pub struct Vfs {
    /// List of VFS layers ordered from highest to lowest priority.
    pub entries: Vec<Arc<dyn VfsLayer + Send + Sync>>,
}

/// A virtual filesystem that mounts other filesystems.
#[derive(Debug)]
pub struct VfsInitError {
    path: PathBuf,
    error: io::Error,
}

static MODS_DIR: &str = "mods";
static DATA_DIR: &str = "data";
static EXTERNALIZED_DIR: &str = "externalized";
static EDITOR_SLF_NAME: &str = "editor.slf";
static ONE_DOT_THIRTEEN_MARKER: &str = "Ja2Set.dat.xml";

impl Vfs {
    /// Creates a new virtual filesystem.
    pub fn new() -> Vfs {
        Vfs::default()
    }

    /// Adds a filesystem layer backed by a filesystem directory.
    /// The added layer will have lowest priority.
    pub fn add_dir(&mut self, path: &Path) -> Result<Arc<dyn VfsLayer>, VfsInitError> {
        let dir_fs = DirFs::new(path).map_err(|error| VfsInitError {
            path: path.to_owned(),
            error,
        })?;
        self.entries.push(dir_fs.clone());
        Ok(dir_fs)
    }

    /// Adds a filesystem layer backed by a SLF file.
    /// The added layer will have lowest priority.
    pub fn add_slf(&mut self, file: Box<dyn VfsFile>) -> Result<Arc<dyn VfsLayer>, VfsInitError> {
        let path = PathBuf::from(format!("{}", file));
        let slf_fs = SlfFs::new(file).map_err(|error| VfsInitError { path, error })?;
        self.entries.push(slf_fs.clone());
        Ok(slf_fs)
    }

    /// Adds a filesystem layer backed by android assets.
    /// The added layer will have lowest priority.
    #[cfg(target_os = "android")]
    pub fn add_android_assets(&mut self, path: &Path) -> Result<Arc<dyn VfsLayer>, VfsInitError> {
        let asset_manager_fs =
            android::AssetManagerFs::new(&path).map_err(|error| VfsInitError {
                path: path.to_owned(),
                error,
            })?;
        self.entries.push(asset_manager_fs.clone());
        Ok(asset_manager_fs)
    }

    /// Adds layers for all SLF files in the passed in layer.
    /// The added layer will have lowest priority.
    pub fn add_slf_files_from(
        &mut self,
        layer: Arc<dyn VfsLayer>,
        required: bool,
    ) -> Result<(), VfsInitError> {
        let slf_paths = layer
            .read_dir_with_extension(&Nfc::caseless_path("/"), &Nfc::caseless("slf"))
            .map_err(|error| VfsInitError {
                path: PathBuf::from(format!("Error listing SLF files in {}", layer)),
                error,
            })?;
        if required && slf_paths.is_empty() {
            return Err(VfsInitError {
                path: PathBuf::from(format!("*.slf in {}", layer)),
                error: ErrorKind::NotFound.into(),
            });
        }
        for path in &slf_paths {
            self.add_slf(layer.open(path).map_err(|error| VfsInitError {
                path: PathBuf::from(format!("{} in {}", path, layer)),
                error,
            })?)?;
        }
        Ok(())
    }

    /// Adds the editor.slf layer to VFS
    fn add_editor_slf_layer(
        &mut self,
        externalized_layer: Arc<dyn VfsLayer>,
    ) -> Result<(), VfsInitError> {
        let editor_slf =
            map_not_found_to_option(externalized_layer.open(&Nfc::caseless_path(EDITOR_SLF_NAME)))
                .map_err(|e| VfsInitError {
                    path: PathBuf::from(format!("{} in {}", EDITOR_SLF_NAME, externalized_layer)),
                    error: e,
                })?;

        if let Some(editor_slf) = editor_slf {
            self.add_slf(editor_slf)?;
        } else {
            warn!(
                "Free editor.slf not found in {}, the editor might not work",
                externalized_layer
            );
        }

        Ok(())
    }

    /// Initializes the VFS overlays from EngineOptions
    pub fn init(
        &mut self,
        engine_options: &EngineOptions,
        mod_manager: &ModManager,
    ) -> Result<(), VfsInitError> {
        let vanilla_game_dir = engine_options.vanilla_game_dir.clone();
        let vanilla_data_dir =
            fs::resolve_existing_components(Path::new(DATA_DIR), Some(&vanilla_game_dir), true);

        let one_dot_thirteen_marker = fs::resolve_existing_components(
            Path::new(ONE_DOT_THIRTEEN_MARKER),
            Some(&vanilla_data_dir),
            true,
        );
        if one_dot_thirteen_marker.exists() {
            log::error!(
                "The game directory seems to be modified by a 1.13 installation, the game might crash at any point in time."
            )
        }

        // First is home data dir (does not need to exist)
        let home_data_dir = fs::resolve_existing_components(
            &PathBuf::from(DATA_DIR),
            Some(&engine_options.stracciatella_home),
            true,
        );
        if home_data_dir.exists() {
            let layer = self.add_dir(&home_data_dir)?;
            // home data dir can include slf files
            self.add_slf_files_from(layer, false)?;
        }

        // Add mod directories
        for mod_id in engine_options.mods.iter().rev() {
            let mod_path = mod_manager
                .get_mod_by_id(mod_id)
                .map(|m| m.path())
                .ok_or_else(|| VfsInitError {
                    path: mod_id.into(),
                    error: ErrorKind::NotFound.into(),
                })?;
            let mod_path = mod_path.join(DATA_DIR);

            match mod_path {
                ModPath::Path(p) => {
                    let p = fs::resolve_existing_components(&p, None, true);
                    let layer = self.add_dir(&p)?;
                    self.add_slf_files_from(layer, false)?;
                }
                #[cfg(target_os = "android")]
                ModPath::AndroidAssetPath(p) => {
                    let layer = android::AssetManagerFs::new(&p).map_err(|e| VfsInitError {
                        path: p.into(),
                        error: e,
                    })?;
                    self.entries.push(layer.clone());
                    self.add_slf_files_from(layer, false)?;
                }
            }
        }

        // Next is externalized data dir (required)
        #[cfg(not(target_os = "android"))]
        let externalized_layer = {
            let externalized_dir = fs::resolve_existing_components(
                Path::new(EXTERNALIZED_DIR),
                Some(&engine_options.assets_dir),
                true,
            );
            self.add_dir(&externalized_dir)
        }?;
        // On android the externalized dir comes from APK assets
        #[cfg(target_os = "android")]
        let externalized_layer = self.add_android_assets(&Path::new(EXTERNALIZED_DIR))?;

        // Next is vanilla data dir (required)
        let data_dir_layer = self.add_dir(&vanilla_data_dir)?;

        // Next are SLF files in vanilla data dir
        self.add_slf_files_from(data_dir_layer, true)?;

        // Last is fallback editor.slf if it exists (does not need to exist)
        if engine_options.run_editor {
            self.add_editor_slf_layer(externalized_layer)?;
        }

        // Print VFS layer to console
        for (index, v) in self.entries.iter().rev().enumerate() {
            info!("VFS layer {}: {}", index + 1, v);
        }

        Ok(())
    }

    /// Opens a file in a specific VFS layer given by its index
    pub fn open_in_layer(
        &self,
        layer_index: usize,
        file_path: &Nfc,
    ) -> io::Result<Box<dyn VfsFile>> {
        if let Some(layer) = self.entries.get(layer_index) {
            let res = layer.open(file_path);
            if res.is_ok() {
                log::debug!(
                    "opened file {} in layer {}",
                    file_path,
                    self.entries.len() - layer_index
                )
            }
            res
        } else {
            Err(io::Error::new(
                io::ErrorKind::InvalidInput,
                "layer index out of range",
            ))
        }
    }

    /// Returns the indexes of the layers that a path exists in
    /// The resulting vector is ordered by the highest priority layer last
    pub fn read_layers(&self, path: &Nfc) -> io::Result<Vec<usize>> {
        let mut result = vec![];

        for (idx, layer) in self.entries.iter().enumerate() {
            if layer.exists(path)? {
                result.push(idx);
            }
        }

        Ok(result)
    }

    /// Opens a json file and applies optional patches on higher priority VFS layers
    pub fn read_patched_json(&self, path: &Nfc) -> io::Result<Value> {
        if path
            .as_str()
            .rsplit('.')
            .next()
            .map(|ext| ext.eq_ignore_ascii_case("json"))
            != Some(true)
        {
            return Err(io::Error::new(
                io::ErrorKind::InvalidInput,
                "patched json must end in .json extension",
            ));
        }
        let patch_path =
            Nfc::caseless_path(&format!("{}.patch.json", &path.as_str()[..path.len() - 5]));
        let file_layers = self.read_layers(path)?;
        let highest_prio_file_layer = if let Some(p) = file_layers.first() {
            Ok(p)
        } else {
            Err(io::Error::new(io::ErrorKind::NotFound, "entity not found"))
        }?;
        let mut value: Value = {
            let mut file = self.open_in_layer(*highest_prio_file_layer, path)?;
            let mut content = String::new();
            file.read_to_string(&mut content)?;
            json::de::from_string(&content).map_err(|e| {
                io::Error::new(
                    io::ErrorKind::InvalidData,
                    format!("failed to deserialize json: {}", e),
                )
            })?
        };

        let patch_layers = self.read_layers(&patch_path)?;
        // Order patches from lowest to highest priority
        for patch_layer in patch_layers.iter().rev() {
            // Ignore patches with lower priority than the highest priority layer
            if patch_layer > highest_prio_file_layer {
                continue;
            }

            let patch_value: Patch = {
                let mut file = self.open_in_layer(*patch_layer, &patch_path)?;
                let mut content = String::new();
                file.read_to_string(&mut content)?;
                json::de::from_string(&content).map_err(|e| {
                    io::Error::new(
                        io::ErrorKind::InvalidData,
                        format!("failed to deserialize json: {}", e),
                    )
                })?
            };

            json_patch::patch(&mut value, &patch_value).map_err(|e| {
                io::Error::new(
                    io::ErrorKind::InvalidData,
                    format!("failed to apply patch to json: {}", e),
                )
            })?;
        }

        Ok(value)
    }
}

/// The merged view of all layers, which is always read-only.
///
/// Only the write methods of a single layer are implemented, so this deliberately keeps the
/// read-only defaults of [`VfsLayer`]. Writing goes to one layer the caller picked, never to
/// whichever layer of a merged view happens to accept writes.
impl VfsLayer for Vfs {
    fn open(&self, file_path: &Nfc) -> io::Result<Box<dyn VfsFile>> {
        for (layer_index, entry) in self.entries.iter().enumerate() {
            let file_result = entry.open(file_path);
            if let Err(err) = &file_result {
                if err.kind() == io::ErrorKind::NotFound {
                    continue;
                }
            } else {
                log::debug!(
                    "opened file {} in layer {}",
                    file_path,
                    self.entries.len() - layer_index
                )
            }
            return file_result;
        }
        Err(io::ErrorKind::NotFound.into())
    }

    fn exists(&self, file_path: &Nfc) -> io::Result<bool> {
        for entry in &self.entries {
            if entry.exists(file_path)? {
                return Ok(true);
            }
        }
        Ok(false)
    }

    fn read_dir(&self, file_path: &Nfc) -> io::Result<BTreeSet<Nfc>> {
        let mut entries = BTreeSet::new();
        for entry in self.entries.iter() {
            let layer_result = entry.read_dir(file_path);
            if let Err(err) = &layer_result {
                if err.kind() == io::ErrorKind::NotFound {
                    continue;
                }
            }
            let layer_result = layer_result?;
            for result in layer_result {
                entries.insert(result);
            }
        }
        if entries.is_empty() {
            Err(io::ErrorKind::NotFound.into())
        } else {
            Ok(entries)
        }
    }

    fn metadata(&self, file_path: &Nfc) -> io::Result<VfsMetadata> {
        for entry in &self.entries {
            match entry.metadata(file_path) {
                Err(err) if err.kind() == ErrorKind::NotFound => continue,
                result => return result,
            }
        }
        Err(ErrorKind::NotFound.into())
    }

    fn resolve_existing_components(&self, file_path: &Nfc) -> Nfc {
        for entry in &self.entries {
            if entry.exists(file_path).unwrap_or(false) {
                return entry.resolve_existing_components(file_path);
            }
        }
        file_path.to_owned()
    }
}

impl fmt::Display for Vfs {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str("Vfs { ")?;
        for entry in &self.entries {
            if f.alternate() {
                f.write_str("\n    ")?;
            }
            write!(f, "{}, ", entry)?;
        }
        if f.alternate() {
            f.write_str("\n}")
        } else {
            f.write_str(" }")
        }
    }
}

impl fmt::Display for VfsInitError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_fmt(format_args!(
            "Error initializing VFS for {:?}: {}",
            self.path, self.error
        ))
    }
}

fn map_not_found_to_option<T>(result: io::Result<T>) -> io::Result<Option<T>> {
    match result {
        Ok(t) => Ok(Some(t)),
        Err(e) => {
            if e.kind() == ErrorKind::NotFound {
                Ok(None)
            } else {
                Err(e)
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use std::io;
    use std::io::{Read, Write};

    use tempfile::{TempDir, tempdir};

    use crate::unicode::Nfc;
    use crate::vfs::dir::DirFs;
    use crate::vfs::slf::SlfFs;
    use crate::vfs::tmp::TmpFs;
    use crate::{config::EngineOptions, mods::ModManager};

    use super::{Vfs, VfsLayer, VfsOpenOptions};

    #[test]
    fn missing_game_data_dir_should_fail() {
        let (engine_options, _temp_dir) = create_test_engine_options();
        let mod_manager = ModManager::new_unchecked(&engine_options);

        let mut vfs = Vfs::new();
        vfs.init(&engine_options, &mod_manager).unwrap_err();
    }

    #[test]
    fn game_data_dir_without_any_slf_files_should_fail() {
        let (engine_options, _temp_dir) = create_test_engine_options();
        let mod_manager = ModManager::new_unchecked(&engine_options);

        let data_path = engine_options.vanilla_game_dir.join("data");
        std::fs::create_dir(&data_path).expect("create `data` dir");

        let mut vfs = Vfs::new();
        vfs.init(&engine_options, &mod_manager).unwrap_err();
    }

    #[test]
    fn game_data_dir_should_be_case_insensitive() {
        let (engine_options, _temp_dir) = create_test_engine_options();
        let mod_manager = ModManager::new_unchecked(&engine_options);

        let data_path = engine_options.vanilla_game_dir.join("data");
        std::fs::create_dir(&data_path).expect("create `data` dir");
        std::fs::write(data_path.join("empty.slf"), EMPTY_SLF_BYTES).expect("write `empty.slf`");

        let mut vfs = Vfs::new();
        vfs.init(&engine_options, &mod_manager).unwrap();

        let (engine_options, _temp_dir) = create_test_engine_options();
        let mod_manager = ModManager::new_unchecked(&engine_options);

        let data_path = engine_options.vanilla_game_dir.join("dAtA");
        std::fs::create_dir(&data_path).expect("create `dAtA` dir");
        std::fs::write(data_path.join("empty.slf"), EMPTY_SLF_BYTES).expect("write `empty.slf`");

        let mut vfs = Vfs::new();
        vfs.init(&engine_options, &mod_manager).unwrap();
    }

    #[test]
    fn missing_mod_data_dir_should_fail() {
        let (mut engine_options, _temp_dir) = create_test_engine_options();
        engine_options.mods = vec!["test-mod".to_owned()];
        let mod_manager = ModManager::new_unchecked(&engine_options);

        let data_path = engine_options.vanilla_game_dir.join("data");
        std::fs::create_dir(&data_path).expect("create `data` dir");
        std::fs::write(data_path.join("empty.slf"), EMPTY_SLF_BYTES).expect("write `empty.slf`");

        let mut vfs = Vfs::new();
        vfs.init(&engine_options, &mod_manager).unwrap_err();
    }

    #[test]
    fn mod_data_dir_should_be_case_insensitive() {
        let (mut engine_options, _temp_dir) = create_test_engine_options();
        engine_options.mods = vec!["test-mod".to_owned()];
        // mod directory has to be created before mod_manager is initialized, so it will pick it up as a mod
        std::fs::create_dir_all(engine_options.stracciatella_home.join("mods/test-mod/data"))
            .expect("create `test-mod/data` dir");
        let mod_manager = ModManager::new_unchecked(&engine_options);

        let data_path = engine_options.vanilla_game_dir.join("data");
        std::fs::create_dir(&data_path).expect("create `data` dir");
        std::fs::write(data_path.join("empty.slf"), EMPTY_SLF_BYTES).expect("write `empty.slf`");

        let mut vfs = Vfs::new();
        vfs.init(&engine_options, &mod_manager).unwrap();

        std::fs::remove_dir_all(engine_options.stracciatella_home.join("mods/test-mod/data"))
            .expect("remove `test-mod/data` dir");
        std::fs::create_dir_all(engine_options.stracciatella_home.join("mods/test-mod/dAtA"))
            .expect("create `test-mod/dAtA` dir");

        let mut vfs = Vfs::new();
        vfs.init(&engine_options, &mod_manager).unwrap();
    }

    #[test]
    fn read_only_layer_should_reject_writes() {
        let temp_dir = tempdir().expect("temp_dir");
        // The directory itself is perfectly writable, the layer is not. Writability is intent.
        let layer = DirFs::new(temp_dir.path()).expect("read only layer");

        let err = layer
            .open_with_options(&Nfc::caseless_path("new.txt"), write_options())
            .unwrap_err();
        assert_eq!(err.kind(), io::ErrorKind::Unsupported);
        assert!(!layer.is_writable());
        assert_eq!(
            layer
                .create_dir(&Nfc::caseless_path("sub"))
                .unwrap_err()
                .kind(),
            io::ErrorKind::Unsupported
        );
        assert!(!temp_dir.path().join("new.txt").exists());
    }

    #[test]
    fn slf_layer_should_reject_writes() {
        let temp_dir = tempdir().expect("temp_dir");
        std::fs::write(temp_dir.path().join("empty.slf"), EMPTY_SLF_BYTES).expect("empty.slf");
        let dir_layer = DirFs::new(temp_dir.path()).expect("dir layer");
        let slf_file = dir_layer
            .open(&Nfc::caseless_path("empty.slf"))
            .expect("open empty.slf");
        let slf_layer = SlfFs::new(slf_file).expect("slf layer");

        assert!(!slf_layer.is_writable());
        let err = slf_layer
            .open_with_options(&Nfc::caseless_path("anything.dat"), write_options())
            .unwrap_err();
        assert_eq!(err.kind(), io::ErrorKind::Unsupported);
    }

    #[test]
    fn vfs_should_never_be_writable() {
        let temp_dir = tempdir().expect("temp_dir");
        let read_only_dir = temp_dir.path().join("read_only");
        let writable_dir = temp_dir.path().join("writable");
        std::fs::create_dir(&read_only_dir).expect("create read_only");
        std::fs::create_dir(&writable_dir).expect("create writable");
        std::fs::write(read_only_dir.join("data.dat"), b"vanilla").expect("write data.dat");

        // Even with a writable layer mounted, the merged view stays read-only. Writes go to a
        // layer the caller picked, never through the Vfs.
        let mut vfs = Vfs::new();
        vfs.entries
            .push(DirFs::new(&read_only_dir).expect("read only layer"));
        vfs.entries
            .push(DirFs::new_writable(&writable_dir).expect("writable layer"));

        assert!(!vfs.is_writable());
        assert_eq!(
            vfs.open_with_options(&Nfc::caseless_path("new.txt"), write_options())
                .unwrap_err()
                .kind(),
            io::ErrorKind::Unsupported
        );
        assert_eq!(
            vfs.create_dir(&Nfc::caseless_path("sub"))
                .unwrap_err()
                .kind(),
            io::ErrorKind::Unsupported
        );
        assert_eq!(
            vfs.remove_file(&Nfc::caseless_path("data.dat"))
                .unwrap_err()
                .kind(),
            io::ErrorKind::Unsupported
        );
        assert_eq!(
            vfs.rename(
                &Nfc::caseless_path("data.dat"),
                &Nfc::caseless_path("moved.dat"),
            )
            .unwrap_err()
            .kind(),
            io::ErrorKind::Unsupported
        );
        assert!(!writable_dir.join("new.txt").exists());
        assert!(read_only_dir.join("data.dat").exists());

        // Reading through the same call still works, it never reaches the read-only error.
        let mut file = vfs
            .open_with_options(&Nfc::caseless_path("data.dat"), read_options())
            .expect("open data.dat for reading");
        let mut content = String::new();
        file.read_to_string(&mut content).expect("read data.dat");
        assert_eq!(content, "vanilla");
    }

    #[test]
    fn writable_dir_layer_should_support_the_full_write_api() {
        let temp_dir = tempdir().expect("temp_dir");
        let layer = DirFs::new_writable(temp_dir.path()).expect("writable layer");

        layer
            .create_dir(&Nfc::caseless_path("saves/backup"))
            .expect("create nested dir");
        assert!(temp_dir.path().join("saves/backup").is_dir());

        {
            let mut file = layer
                .open_with_options(&Nfc::caseless_path("saves/one.sav"), write_options())
                .expect("create one.sav");
            file.write_all(b"12345").expect("write one.sav");
        }
        assert_eq!(
            layer
                .metadata(&Nfc::caseless_path("saves/one.sav"))
                .expect("metadata")
                .len,
            5
        );
        assert!(
            layer
                .metadata(&Nfc::caseless_path("saves"))
                .expect("metadata")
                .is_dir
        );

        // Case insensitive resolution has to keep working for paths written through the layer.
        assert!(
            layer
                .exists(&Nfc::caseless_path("SAVES/ONE.SAV"))
                .expect("exists")
        );
        assert_eq!(
            layer
                .find_all_files_in_dir(&Nfc::caseless_path("saves"), false)
                .expect("find files"),
            vec![Nfc::caseless_path("one.sav")]
        );

        layer
            .rename(
                &Nfc::caseless_path("saves/one.sav"),
                &Nfc::caseless_path("saves/two.sav"),
            )
            .expect("rename");
        assert!(temp_dir.path().join("saves/two.sav").exists());
        assert!(!temp_dir.path().join("saves/one.sav").exists());

        layer
            .erase_dir(&Nfc::caseless_path("saves"))
            .expect("erase_dir");
        assert!(!temp_dir.path().join("saves/two.sav").exists());
        // erase_dir leaves subdirectories alone.
        assert!(temp_dir.path().join("saves/backup").is_dir());
    }

    #[test]
    fn writable_dir_layer_should_keep_the_case_of_created_paths() {
        let temp_dir = tempdir().expect("temp_dir");
        let layer = DirFs::new_writable(temp_dir.path()).expect("writable layer");

        // A file has to land on disk under the name it was asked for, even though looking it up
        // afterwards is case insensitive.
        layer
            .create_dir(&Nfc::path("SavedGames"))
            .expect("create SavedGames");
        {
            let mut file = layer
                .open_with_options(&Nfc::path("SavedGames/SaveGame01.sav"), write_options())
                .expect("create SaveGame01.sav");
            file.write_all(b"savegame").expect("write SaveGame01.sav");
        }

        assert!(temp_dir.path().join("SavedGames/SaveGame01.sav").is_file());
        assert!(
            layer
                .exists(&Nfc::caseless_path("savedgames/savegame01.sav"))
                .expect("exists")
        );
        assert_eq!(
            layer.resolve_existing_components(&Nfc::caseless_path("savedgames/savegame01.sav")),
            Nfc::path("SavedGames/SaveGame01.sav")
        );
        // Listings fold the case like every other VFS lookup, resolving is what gets the name on
        // disk back.
        assert_eq!(
            layer
                .find_all_dirs_in_dir(&Nfc::caseless(""), false)
                .expect("find dirs"),
            vec![Nfc::caseless_path("savedgames")]
        );
        assert_eq!(
            layer
                .find_all_files_in_dir(&Nfc::caseless(""), true)
                .expect("find files"),
            vec![Nfc::caseless_path("savedgames/savegame01.sav")]
        );
        // A component that does not exist keeps the case it was asked for.
        assert_eq!(
            layer.resolve_existing_components(&Nfc::path("savedgames/Missing.sav")),
            Nfc::path("SavedGames/Missing.sav")
        );
    }

    #[test]
    fn creating_nested_dirs_should_invalidate_the_whole_cache_path() {
        let temp_dir = tempdir().expect("temp_dir");
        let layer = DirFs::new_writable(temp_dir.path()).expect("writable layer");

        // Populates the cache with a listing of the still empty root.
        assert!(!layer.exists(&Nfc::path("saves/backup")).expect("exists"));

        layer
            .create_dir(&Nfc::path("saves/backup"))
            .expect("create nested dirs");

        // Both levels were created at once, so both cached listings had to be dropped.
        assert!(
            layer
                .metadata(&Nfc::path("saves"))
                .expect("metadata")
                .is_dir
        );
        assert!(
            layer
                .metadata(&Nfc::path("saves/backup"))
                .expect("metadata")
                .is_dir
        );
    }

    #[test]
    fn tmp_layer_should_be_writable_and_own_its_directory() {
        let layer = TmpFs::new().expect("tmp layer");
        let path = layer.path().to_owned();
        assert!(path.is_dir());
        assert!(layer.is_writable());

        {
            let mut file = layer
                .open_with_options(&Nfc::path("scratch.dat"), write_options())
                .expect("create scratch.dat");
            file.write_all(b"scratch").expect("write scratch.dat");
        }
        assert!(path.join("scratch.dat").is_file());

        // The layer owns the directory, so dropping it takes the contents with it.
        drop(layer);
        assert!(!path.exists());
    }

    /// Options equivalent to what `FileMan::openForReading` passes.
    fn read_options() -> VfsOpenOptions {
        VfsOpenOptions {
            read: true,
            ..Default::default()
        }
    }

    /// Options equivalent to what `FileMan::openForWriting` passes.
    fn write_options() -> VfsOpenOptions {
        VfsOpenOptions {
            write: true,
            truncate: true,
            create: true,
            ..Default::default()
        }
    }

    const EMPTY_SLF_BYTES: &[u8] = include_bytes!("test_fixtures/empty.slf");

    fn create_test_engine_options() -> (EngineOptions, TempDir) {
        let temp_dir = tempdir().expect("temp_dir");
        let mut engine_options = EngineOptions::default();

        let home_dir = temp_dir.path().join("home");
        let assets_dir = temp_dir.path().join("assets");
        let vanilla_game_dir = temp_dir.path().join("game_dir");

        std::fs::create_dir(&home_dir).expect("home_dir");
        std::fs::create_dir_all(assets_dir.join("externalized")).expect("assets_dir");
        std::fs::create_dir(&vanilla_game_dir).expect("vanilla_game_dir");

        engine_options.stracciatella_home = home_dir;
        engine_options.assets_dir = assets_dir;
        engine_options.vanilla_game_dir = vanilla_game_dir;

        (engine_options, temp_dir)
    }
}
