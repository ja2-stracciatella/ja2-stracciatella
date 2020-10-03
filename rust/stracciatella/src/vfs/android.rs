//! This module contains a virtual filesystem backed by a SLF file.
#![allow(dead_code)]

use std::ffi::CString;
use std::fmt;
use std::io;
use std::io::SeekFrom;
use std::path::{Path, PathBuf};

use ndk::asset::{Asset, AssetManager};

use crate::android::get_asset_manager;
use crate::unicode::Nfc;

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
    pub fn new(base_path: &Path) -> io::Result<AssetManagerFs> {
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
        Ok(AssetManagerFs {
            base_path: base_path.to_owned(),
            asset_manager,
        })
    }

    /// Opens a file in the filesystem.
    /// This is currently very basic and not case insensitive
    pub fn open(&self, file_path: &Nfc) -> io::Result<AssetManagerFsFile> {
        let asset_path = Self::path_to_cstring(&self.base_path.join(file_path.as_str()))?;
        let asset = self.asset_manager.open(&asset_path).ok_or_else(|| {
            io::Error::new(
                io::ErrorKind::NotFound,
                format!("AssetManagerFs: Asset not found: `{:?}`", asset_path),
            )
        })?;

        Ok(AssetManagerFsFile {
            file_path: file_path.clone(),
            base_path: self.base_path.clone(),
            file: asset,
        })
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

impl AssetManagerFsFile {
    /// Gets the length of the file.
    pub fn len(&self) -> io::Result<u64> {
        Ok(self.file.get_length() as u64)
    }

    /// Returns true if the file is empty.
    pub fn is_empty(&self) -> io::Result<bool> {
        self.len().map(|x| x == 0)
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
