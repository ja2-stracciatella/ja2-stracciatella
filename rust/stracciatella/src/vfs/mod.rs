//! This module implements virtual filesystems.
//!
//! The paths are case insensitive.
//! It does not support path components `.` and `..`.
#![allow(dead_code)]

#[cfg(target_os = "android")]
pub mod android;
pub mod dir;
pub mod slf;

use std::fmt;
use std::io;
use std::io::ErrorKind;
use std::path::{Path, PathBuf};
use std::rc::Rc;
use std::collections::HashSet;

use log::{info, warn};

use crate::unicode::Nfc;
use crate::vfs::dir::DirFs;
use crate::vfs::slf::SlfFs;
use crate::EngineOptions;
use crate::{fs, get_assets_dir};

pub trait VfsFile: io::Read + io::Seek + io::Write + fmt::Debug + fmt::Display {
    /// Returns the length of the file
    fn len(&self) -> io::Result<u64>;

    /// Returns true if the virtual file is empty.
    fn is_empty(&self) -> io::Result<bool> {
        Ok(self.len()? == 0)
    }
}

pub trait VfsLayer: fmt::Debug + fmt::Display {
    // Opens a file in the VFS Layer
    fn open(&self, file_path: &Nfc) -> io::Result<Box<dyn VfsFile>>;
    // Lists a directory in the VFS Layer
    fn read_dir(&self, file_path: &Nfc) -> io::Result<HashSet<Nfc>>;
}

/// A virtual filesystem that mounts other filesystems.
#[derive(Debug, Default)]
pub struct Vfs {
    /// List of entries.
    pub entries: Vec<Rc<dyn VfsLayer>>,
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

impl Vfs {
    /// Creates a new virtual filesystem.
    pub fn new() -> Vfs {
        Vfs::default()
    }

    /// Adds an overlay filesystem backed by a filesystem directory.
    pub fn add_dir(&mut self, path: &Path) -> Result<Rc<dyn VfsLayer>, VfsInitError> {
        let dir_fs = DirFs::new(&path).map_err(|error| VfsInitError {
            path: path.to_owned(),
            error,
        })?;
        self.entries.push(dir_fs.clone());
        Ok(dir_fs)
    }

    /// Adds an overlay filesystem backed by a SLF file.
    pub fn add_slf(&mut self, path: &Path) -> Result<Rc<dyn VfsLayer>, VfsInitError> {
        let slf_fs = SlfFs::new(&path).map_err(|error| VfsInitError {
            path: path.to_owned(),
            error,
        })?;
        self.entries.push(slf_fs.clone());
        Ok(slf_fs)
    }

    /// Adds an overlay filesystem backed by android assets
    #[cfg(target_os = "android")]
    pub fn add_android_assets(&mut self, path: &Path) -> Result<Rc<dyn VfsLayer>, VfsInitError> {
        let asset_manager_fs =
            android::AssetManagerFs::new(&path).map_err(|error| VfsInitError {
                path: path.to_owned(),
                error,
            })?;
        self.entries.push(asset_manager_fs.clone());
        Ok(asset_manager_fs)
    }

    /// Adds an overlay for all SLF files in dir
    pub fn add_slf_files(&mut self, path: &Path, required: bool) -> Result<(), VfsInitError> {
        let slf_paths = fs::read_dir_paths(path, false).map_err(|error| VfsInitError {
            path: path.to_owned(),
            error,
        })?;
        let slf_paths: Vec<_> = slf_paths
            .iter()
            .filter(|path| {
                path.extension().map(|e| e.to_string_lossy().to_lowercase())
                    == Some("slf".to_string())
            })
            .collect();
        if required && slf_paths.is_empty() {
            return Err(VfsInitError {
                path: path.join(Path::new("*.slf")),
                error: ErrorKind::NotFound.into(),
            });
        }
        for path in slf_paths {
            self.add_slf(&path)?;
        }
        Ok(())
    }

    /// Initializes the VFS overlays from EngineOptions
    pub fn init_from_engine_options(
        &mut self,
        engine_options: &EngineOptions,
    ) -> Result<(), VfsInitError> {
        let vanilla_game_dir = engine_options.vanilla_game_dir.clone();
        let vanilla_data_dir =
            fs::resolve_existing_components(Path::new(DATA_DIR), Some(&vanilla_game_dir), true);
        let assets_dir = fs::resolve_existing_components(&get_assets_dir(), None, true);
        let home_data_dir = fs::resolve_existing_components(
            &PathBuf::from(DATA_DIR),
            Some(&engine_options.stracciatella_home),
            true,
        );
        let externalized_dir =
            fs::resolve_existing_components(Path::new(EXTERNALIZED_DIR), Some(&assets_dir), true);
        let editor_slf_path = fs::resolve_existing_components(
            &Path::new(EDITOR_SLF_NAME),
            Some(&externalized_dir),
            true,
        );

        // Add mod directories
        for mod_name in engine_options.mods.iter() {
            // First are mod directories in home directory then mods in externalized directory (only one of them is required)
            let mod_path =
                Path::new(MODS_DIR).join(Path::new(&format!("{}/{}", &mod_name, DATA_DIR)));
            let mod_in_home = fs::resolve_existing_components(
                &mod_path,
                Some(&engine_options.stracciatella_home),
                true,
            );
            let mod_in_externalized =
                fs::resolve_existing_components(&mod_path, Some(&assets_dir), true);

            match (mod_in_home.exists(), mod_in_externalized.exists()) {
                (false, false) => {
                    return Err(VfsInitError {
                        path: mod_path,
                        error: ErrorKind::NotFound.into(),
                    });
                }
                (true, false) => {
                    self.add_dir(&mod_in_home)?;
                    self.add_slf_files(&mod_in_home, false)?;
                }
                (false, true) => {
                    self.add_dir(&mod_in_externalized)?;
                    self.add_slf_files(&mod_in_externalized, false)?;
                }
                (true, true) => {
                    self.add_dir(&mod_in_home)?;
                    self.add_slf_files(&mod_in_home, false)?;
                    self.add_dir(&mod_in_externalized)?;
                    self.add_slf_files(&mod_in_externalized, false)?;
                }
            };
        }

        // Next is home data dir (does not need to exist)
        if home_data_dir.exists() {
            self.add_dir(&home_data_dir)?;
            // home data dir can include slf files
            self.add_slf_files(&home_data_dir, false)?;
        }

        // Next is externalized data dir (required)
        #[cfg(not(target_os = "android"))]
        self.add_dir(&externalized_dir)?;
        // On android the externalized dir comes from APK assets
        #[cfg(target_os = "android")]
        self.add_android_assets(&Path::new(EXTERNALIZED_DIR))?;

        // Next is vanilla data dir (required)
        self.add_dir(&vanilla_data_dir)?;

        // Next are SLF files in vanilla data dir
        self.add_slf_files(&vanilla_data_dir, true)?;

        // Last is fallback editor.slf if it exists (does not need to exist)
        if engine_options.run_editor {
            if editor_slf_path.exists() {
                self.add_slf(&editor_slf_path)?;
            } else {
                warn!(
                    "Free editor.slf not found in {:?}, the editor might not work",
                    assets_dir
                );
            }
        }

        // Print VFS order to console
        for (index, v) in self.entries.iter().enumerate() {
            info!(
                "VFS item with priority {}: {}",
                self.entries.len() - index,
                v
            );
        }

        Ok(())
    }
}

impl VfsLayer for Vfs {
    fn open(&self, file_path: &Nfc) -> io::Result<Box<dyn VfsFile>> {
        for entry in self.entries.iter() {
            let file_result = entry.open(&file_path);
            if let Err(err) = &file_result {
                if err.kind() == io::ErrorKind::NotFound {
                    continue;
                }
            }
            return file_result;
        }
        Err(io::ErrorKind::NotFound.into())
    }

    fn read_dir(&self, file_path: &Nfc) -> io::Result<HashSet<Nfc>> {
        let mut entries = HashSet::new();
        for entry in self.entries.iter() {
            let layer_result = entry.read_dir(&file_path);
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
