//! This module contains a virtual filesystem backed by a SLF file.
#![allow(dead_code)]

use std::ffi::CString;
use std::fmt;
use std::io;
use std::io::SeekFrom;
use std::path::{Path, PathBuf};
use std::rc::Rc;
use std::collections::HashSet;

use ndk::asset::{Asset, AssetManager};

use crate::android::get_asset_manager;
use crate::unicode::Nfc;
use crate::vfs::{VfsFile, VfsLayer};

/// A case-insensitive virtual filesystem backed by a filesystem directory.
#[derive(Debug)]
pub struct AssetManagerFs {
    /// Which base directory used from assets
    pub base_path: PathBuf,
    /// A local reference to the android asset manager
    asset_manager: AssetManager,
}

/// A virtual file.
#[derive(Debug)]
pub struct AssetManagerFsFile {
    /// Display info.
    pub file_path: Nfc,
    /// Display info.
    pub base_path: PathBuf,
    /// Asset that is open.
    pub file: Asset,
}

impl AssetManagerFs {
    /// Creates a new virtual filesystem.
    pub fn new(base_path: &Path) -> io::Result<Rc<AssetManagerFs>> {
        let asset_manager = get_asset_manager().map_err(|err| {
            io::Error::new(
                io::ErrorKind::Other,
                format!(
                    "AssetManagerFs: Error getting Android AssetManager `{:?}`",
                    err
                ),
            )
        })?;
        let asset_path = Self::path_to_cstring(base_path)?;
        asset_manager.open_dir(&asset_path).ok_or_else(|| {
            io::Error::new(
                io::ErrorKind::NotFound,
                format!("AssetManagerFs: Error testing base path `{:?}`", base_path),
            )
        })?;
        Ok(Rc::new(AssetManagerFs {
            base_path: base_path.to_owned(),
            asset_manager,
        }))
    }

    /// Maps a path to all candidates that might match the path case insensitively
    ///
    /// The returned paths are already containing the base path
    fn canonicalize(&self, file_path: &str) -> io::Result<Vec<PathBuf>> {
        let mut candidates = vec![self.base_path.to_owned()];

        if file_path.is_empty() {
            return Ok(candidates);
        }

        for want in file_path.split('/') {
            let mut next = Vec::new();
            if want == "." || want == ".." {
                return Err(io::Error::new(
                    io::ErrorKind::InvalidInput,
                    "special path components are not supported",
                ));
            }
            for candidate in candidates {
                let candidate_cstring = Self::path_to_cstring(&candidate)?;
                let dir_contents = crate::android::list_asset_dir(&candidate).map_err(|e| {
                    io::Error::new(
                        io::ErrorKind::Other,
                        format!("AssetManagerFs: JNI Error: `{:?}`", e),
                    )
                })?;
                if self.asset_manager.open_dir(&candidate_cstring).is_some() {
                    for entry in dir_contents {
                        let is_match = entry
                            .file_name()
                            .and_then(|x| x.to_str())
                            .map(|x| want == Nfc::caseless(x).as_str())
                            .unwrap_or(false);
                        if is_match {
                            next.push(candidate.join(&entry));
                        }
                    }
                }
            }
            candidates = next;
            if candidates.is_empty() {
                break;
            }
        }
        candidates.sort();

        Ok(candidates)
    }

    /// Maps a path to CString for asset manager
    fn path_to_cstring(path: &Path) -> io::Result<CString> {
        let path = path
            .to_owned()
            .into_os_string()
            .into_string()
            .map_err(|err| {
                io::Error::new(
                    io::ErrorKind::InvalidInput,
                    format!(
                        "Could not convert path to string for AssetManager: {:?}",
                        err
                    ),
                )
            })?;
        CString::new(path.as_bytes()).map_err(|err| {
            io::Error::new(
                io::ErrorKind::InvalidInput,
                format!(
                    "Could not convert path to string for AssetManager: {:?}",
                    err
                ),
            )
        })
    }
}

impl VfsLayer for AssetManagerFs {
    fn open(&self, file_path: &Nfc) -> io::Result<Box<dyn VfsFile>> {
        let candidates = self.canonicalize(file_path)?;

        for candidate in candidates {
            let candidate_cstring = Self::path_to_cstring(&candidate)?;
            if let Some(file) = self.asset_manager.open(&candidate_cstring) {
                return Ok(Box::new(AssetManagerFsFile {
                    file_path: file_path.clone(),
                    base_path: self.base_path.clone(),
                    file,
                }));
            }
        }

        Err(io::Error::new(
            io::ErrorKind::NotFound,
            format!(
                "AssetManagerFs: Asset not found: `{:?}`",
                self.base_path.join(&file_path.as_str())
            ),
        ))
    }

    fn read_dir(&self, file_path: &Nfc) -> io::Result<HashSet<Nfc>> {
        let file_path = file_path.trim_end_matches('/');
        let candidates = self.canonicalize(file_path)?;
        let mut result = HashSet::new();

        for candidate in candidates {
            let dir_contents = crate::android::list_asset_dir(&candidate).map_err(|e| {
                io::Error::new(
                    io::ErrorKind::Other,
                    format!("AssetManagerFs: JNI Error: `{:?}`", e),
                )
            })?;

            for file_name in dir_contents {
                let file_name_nfc =
                    Nfc::caseless_path(&file_name.into_os_string().into_string().map_err(|err| {
                        io::Error::new(
                            io::ErrorKind::InvalidInput,
                            format!("Could not convert path to NFC for AssetManager: {:?}", err),
                        )
                    })?);
                result.insert(file_name_nfc);
            }
        }

        Ok(result)
    }
}

impl VfsFile for AssetManagerFsFile {
    /// Gets the length of the file.
    fn len(&self) -> io::Result<u64> {
        Ok(self.file.get_length() as u64)
    }
}

impl fmt::Display for AssetManagerFs {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "AssetManagerFs {{ {:?} }}", self.base_path)
    }
}

impl fmt::Display for AssetManagerFsFile {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "AssetManagerFsFile {{ {:?} in {:?} }}",
            self.file_path, self.base_path
        )
    }
}

impl io::Read for AssetManagerFsFile {
    fn read(&mut self, buf: &mut [u8]) -> io::Result<usize> {
        self.file.read(buf)
    }
}

impl io::Seek for AssetManagerFsFile {
    fn seek(&mut self, pos: SeekFrom) -> io::Result<u64> {
        self.file.seek(pos)
    }
}

impl io::Write for AssetManagerFsFile {
    fn write(&mut self, _buf: &[u8]) -> io::Result<usize> {
        Err(io::Error::new(
            io::ErrorKind::PermissionDenied,
            "stracciatella::vfs::slf::AssetManagerFsFile is read-only",
        ))
    }
    fn flush(&mut self) -> io::Result<()> {
        Err(io::Error::new(
            io::ErrorKind::PermissionDenied,
            "stracciatella::vfs::slf::AssetManagerFsFile is read-only",
        ))
    }
}
