//! This module implements virtual filesystems.
//!
//! The paths are case insensitive.
//! It does not support path components `.` and `..`.
#![allow(dead_code)]

#[cfg(target_os = "android")]
pub mod android;
pub mod dir;
pub mod slf;

use std::collections::BTreeSet;
use std::fmt;
use std::io;
use std::io::ErrorKind;
use std::path::{Path, PathBuf};
use std::sync::Arc;

use json_patch::Patch;
use log::{info, warn};
use serde_json::Value;

use crate::fs;
use crate::json;
use crate::mods::ModManager;
use crate::mods::ModPath;
use crate::unicode::Nfc;
use crate::vfs::dir::DirFs;
use crate::vfs::slf::SlfFs;
use crate::EngineOptions;

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
            log::error!("The game directory seems to be modified by a 1.13 installation, the game might crash at any point in time.")
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
    use tempfile::{tempdir, TempDir};

    use crate::{config::EngineOptions, mods::ModManager};

    use super::Vfs;

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
        std::fs::write(&data_path.join("empty.slf"), EMPTY_SLF_BYTES).expect("write `empty.slf`");

        let mut vfs = Vfs::new();
        vfs.init(&engine_options, &mod_manager).unwrap();

        let (engine_options, _temp_dir) = create_test_engine_options();
        let mod_manager = ModManager::new_unchecked(&engine_options);

        let data_path = engine_options.vanilla_game_dir.join("dAtA");
        std::fs::create_dir(&data_path).expect("create `dAtA` dir");
        std::fs::write(&data_path.join("empty.slf"), EMPTY_SLF_BYTES).expect("write `empty.slf`");

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
        std::fs::write(&data_path.join("empty.slf"), EMPTY_SLF_BYTES).expect("write `empty.slf`");

        let mut vfs = Vfs::new();
        vfs.init(&engine_options, &mod_manager).unwrap_err();
    }

    #[test]
    fn mod_data_dir_should_be_case_insensitive() {
        let (mut engine_options, _temp_dir) = create_test_engine_options();
        engine_options.mods = vec!["test-mod".to_owned()];
        // mod directory has to be created before mod_manager is initialized, so it will pick it up as a mod
        std::fs::create_dir_all(&engine_options.stracciatella_home.join("mods/test-mod/data"))
            .expect("create `test-mod/data` dir");
        let mod_manager = ModManager::new_unchecked(&engine_options);

        let data_path = engine_options.vanilla_game_dir.join("data");
        std::fs::create_dir(&data_path).expect("create `data` dir");
        std::fs::write(&data_path.join("empty.slf"), EMPTY_SLF_BYTES).expect("write `empty.slf`");

        let mut vfs = Vfs::new();
        vfs.init(&engine_options, &mod_manager).unwrap();

        std::fs::remove_dir_all(&engine_options.stracciatella_home.join("mods/test-mod/data"))
            .expect("remove `test-mod/data` dir");
        std::fs::create_dir_all(&engine_options.stracciatella_home.join("mods/test-mod/dAtA"))
            .expect("create `test-mod/dAtA` dir");

        let mut vfs = Vfs::new();
        vfs.init(&engine_options, &mod_manager).unwrap();
    }

    const EMPTY_SLF_BYTES: &[u8] = include_bytes!("test_fixtures/empty.slf");

    fn create_test_engine_options() -> (EngineOptions, TempDir) {
        let temp_dir = tempdir().expect("temp_dir");
        let mut engine_options = EngineOptions::default();

        let home_dir = temp_dir.path().join("home");
        let assets_dir = temp_dir.path().join("assets");
        let vanilla_game_dir = temp_dir.path().join("game_dir");

        std::fs::create_dir(&home_dir).expect("home_dir");
        std::fs::create_dir_all(&assets_dir.join("externalized")).expect("assets_dir");
        std::fs::create_dir(&vanilla_game_dir).expect("vanilla_game_dir");

        engine_options.stracciatella_home = home_dir;
        engine_options.assets_dir = assets_dir;
        engine_options.vanilla_game_dir = vanilla_game_dir;

        (engine_options, temp_dir)
    }
}
