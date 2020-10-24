//! This module contains a virtual filesystem backed by the Android Asset Manager
//!
//! All paths opened in this VFS are relative to the base asset directory passed in `new`. They should
//! not start with a `/`.
//!
//! ## Assumptions
//!
//! There are some assumptions we make about the AssetManager in oder to make everything work smoothly.
//!
//! - The directory and file structure within Android assets does not change during a single run of the game
//! - All paths within the Android assets can be de- and encoded to UTF-8
//!
#![allow(dead_code)]

use std::collections::HashSet;
use std::ffi::CString;
use std::ffi::OsString;
use std::fmt;
use std::io;
use std::io::SeekFrom;
use std::path::{Path, PathBuf};
use std::rc::Rc;
use std::sync::Mutex;

use lru::LruCache;
use ndk::asset::{Asset, AssetManager};

use crate::android::get_asset_manager;
use crate::unicode::Nfc;
use crate::vfs::{VfsFile, VfsLayer};

/// The size of the cache used for canonicalization
const CANONICALIZATION_CACHE_SIZE: usize = 256;

/// A case-insensitive virtual filesystem backed by a filesystem directory.
#[derive(Debug)]
pub struct AssetManagerFs {
    /// Which base directory used from assets
    pub base_path: PathBuf,
    /// A local reference to the android asset manager
    asset_manager: AssetManager,
    /// Cache that is used for canonicalization. It will contain an entry for each path that is listed during path canonicalization
    canonicalization_cache: Mutex<LruCache<PathBuf, Vec<(Nfc, OsString)>>>,
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
    ///
    /// Note: The base_path will not be canonicalized. This means it has to exist exactly as specified in `new`.
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
            canonicalization_cache: Mutex::new(LruCache::new(CANONICALIZATION_CACHE_SIZE)),
        }))
    }

    /// Maps a path to all candidates that might match the path case insensitively
    ///
    /// The returned paths are already containing the base path
    fn canonicalize(&self, file_path: &str) -> io::Result<Vec<PathBuf>> {
        let mut candidates = vec![self.base_path.to_owned()];
        let mut canonicalization_cache = self.canonicalization_cache.lock().map_err(|err| {
            io::Error::new(
                io::ErrorKind::Other,
                format!(
                    "AssetManagerFs: Error locking canonicalization cache: `{}`",
                    err
                ),
            )
        })?;

        if file_path.is_empty() {
            return Ok(candidates);
        }

        for want in file_path.split('/') {
            let mut next = Vec::new();
            if want == "." || want == ".." {
                return Err(io::Error::new(
                    io::ErrorKind::InvalidInput,
                    "AssetManagerFs: Special path components are not supported",
                ));
            }
            for candidate in candidates {
                let candidate_cstring = Self::path_to_cstring(&candidate)?;
                let entries = if let Some(cache_entry) = canonicalization_cache.get(&candidate) {
                    cache_entry
                } else if self.asset_manager.open_dir(&candidate_cstring).is_none() {
                    canonicalization_cache.put(candidate.clone(), vec![]);
                    canonicalization_cache
                        .get(&candidate)
                        .expect("we should be able to get a cache key that was just set")
                } else {
                    let entries: Vec<_> = crate::android::list_asset_dir(&candidate)
                        .map_err(|e| {
                            io::Error::new(
                                io::ErrorKind::Other,
                                format!("AssetManagerFs: JNI Error: `{:?}`", e),
                            )
                        })?
                        .iter()
                        .flat_map(|path_buf| {
                            path_buf
                                .to_str()
                                .map(|s| (Nfc::caseless(s), path_buf.into()))
                        })
                        .collect();
                    canonicalization_cache.put(candidate.clone(), entries);
                    canonicalization_cache
                        .get(&candidate)
                        .expect("we should be able to get a cache key that was just set")
                };

                for (nfc, os_string) in entries {
                    if want == nfc.as_str() {
                        next.push(candidate.join(&os_string));
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
                let file_name_nfc = Nfc::caseless_path(
                    &file_name.into_os_string().into_string().map_err(|err| {
                        io::Error::new(
                            io::ErrorKind::InvalidInput,
                            format!("Could not convert path to NFC for AssetManager: {:?}", err),
                        )
                    })?,
                );
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
