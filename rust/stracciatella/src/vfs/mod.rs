//! This module implements virtual filesystems.
//!
//! The paths are case insensitive.
//! It does not support path components `.` and `..`.
#![allow(dead_code)]

pub mod dir;
pub mod slf;

use std::fmt;
use std::io;
use std::io::{ErrorKind, SeekFrom};
use std::path::{Path, PathBuf};

use log::{info, warn};

use crate::unicode::Nfc;
use crate::vfs::dir::{DirFs, DirFsFile};
use crate::vfs::slf::{SlfFs, SlfFsFile};
use crate::EngineOptions;
use crate::{fs, get_assets_dir};

/// A virtual filesystem that mounts other filesystems.
#[derive(Debug, Default)]
pub struct Vfs {
    /// List of entries.
    pub entries: Vec<VfsSource>,
}

/// A virtual filesystem that mounts other filesystems.
#[derive(Debug)]
pub struct VfsInitError {
    path: PathBuf,
    error: io::Error,
}

/// A filesystem entry.
#[derive(Debug)]
pub struct VfsEntry {
    pub order: i32,
    pub source: VfsSource,
}

/// A source filesystem.
#[derive(Debug)]
pub enum VfsSource {
    Dir(DirFs),
    Slf(SlfFs),
}

/// A virtual file.
#[derive(Debug)]
pub enum VfsFile {
    Dir(DirFsFile),
    Slf(SlfFsFile),
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
    pub fn add_dir(&mut self, path: &Path) -> Result<(), VfsInitError> {
        let dir_fs = DirFs::new(&path).map_err(|error| VfsInitError {
            path: path.to_owned(),
            error,
        })?;
        self.entries.push(VfsSource::Dir(dir_fs));
        Ok(())
    }

    /// Adds an overlay filesystem backed by a SLF file.
    pub fn add_slf(&mut self, path: &Path) -> Result<(), VfsInitError> {
        let slf_fs = SlfFs::new(&path).map_err(|error| VfsInitError {
            path: path.to_owned(),
            error,
        })?;
        self.entries.push(VfsSource::Slf(slf_fs));
        Ok(())
    }

    /// Adds an overlay for all SLF files in dir
    pub fn add_slf_files(&mut self, path: &Path, required: bool) -> Result<(), VfsInitError> {
        let slf_paths =
            fs::read_dir_paths(path, false).map_err(|error| VfsInitError {
                path: path.to_owned(),
                error,
            })?;
        let slf_paths: Vec<_> = slf_paths.iter().filter(|path| {
            path.extension().map(|e| e.to_string_lossy().to_lowercase()) == Some("slf".to_string())
        }).collect();
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
        let vanilla_data_dir = fs::resolve_existing_components(
            Path::new(DATA_DIR),
            Some(&engine_options.vanilla_game_dir),
            true,
        );
        let assets_dir = fs::resolve_existing_components(&get_assets_dir(), None, true);
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

        // Next is externalized data dir (required)
        self.add_dir(&externalized_dir)?;

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

    /// Opens a file.
    pub fn open(&self, file_path: &Nfc) -> io::Result<VfsFile> {
        for entry in self.entries.iter() {
            let file_result = match &entry {
                VfsSource::Dir(x) => x.open(&file_path).map(VfsFile::Dir),
                VfsSource::Slf(x) => x.open(&file_path).map(VfsFile::Slf),
            };
            if let Err(err) = &file_result {
                if err.kind() == io::ErrorKind::NotFound {
                    continue;
                }
            }
            return file_result;
        }
        Err(io::ErrorKind::NotFound.into())
    }
}

impl VfsFile {
    /// Gets the length of the virtual file.
    pub fn len(&self) -> io::Result<u64> {
        match self {
            VfsFile::Dir(x) => x.len(),
            VfsFile::Slf(x) => Ok(x.len()),
        }
    }

    /// Returns true if the virtual file is empty.
    pub fn is_empty(&self) -> io::Result<bool> {
        match self {
            VfsFile::Dir(x) => x.is_empty(),
            VfsFile::Slf(x) => Ok(x.is_empty()),
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

impl fmt::Display for VfsSource {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            VfsSource::Dir(x) => x.fmt(f),
            VfsSource::Slf(x) => x.fmt(f),
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

impl fmt::Display for VfsFile {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            VfsFile::Dir(x) => x.fmt(f),
            VfsFile::Slf(x) => x.fmt(f),
        }
    }
}

impl io::Read for VfsFile {
    fn read(&mut self, buf: &mut [u8]) -> io::Result<usize> {
        match self {
            VfsFile::Dir(x) => x.read(buf),
            VfsFile::Slf(x) => x.read(buf),
        }
    }
}

impl io::Seek for VfsFile {
    fn seek(&mut self, pos: SeekFrom) -> io::Result<u64> {
        match self {
            VfsFile::Dir(x) => x.seek(pos),
            VfsFile::Slf(x) => x.seek(pos),
        }
    }
}

impl io::Write for VfsFile {
    fn write(&mut self, buf: &[u8]) -> io::Result<usize> {
        match self {
            VfsFile::Dir(x) => x.write(buf),
            VfsFile::Slf(x) => x.write(buf),
        }
    }
    fn flush(&mut self) -> io::Result<()> {
        match self {
            VfsFile::Dir(x) => x.flush(),
            VfsFile::Slf(x) => x.flush(),
        }
    }
}
