//! This module contains a virtual filesystem backed by a SLF file.
#![allow(dead_code)]

use lru::LruCache;
use std::collections::HashSet;
use std::ffi::OsString;
use std::fmt;
use std::io;
use std::io::SeekFrom;
use std::path::{Path, PathBuf};
use std::sync::Arc;
use std::sync::Mutex;

use crate::fs;
use crate::fs::File;
use crate::unicode::Nfc;
use crate::vfs::{VfsFile, VfsLayer};

/// The size of the cache used for canonicalization
const CANONICALIZATION_CACHE_SIZE: usize = 256;

/// A case-insensitive virtual filesystem backed by a filesystem directory.
#[derive(Debug)]
pub struct DirFs {
    /// Path to the directory.
    pub dir_path: PathBuf,
    /// Cache that is used for canonicalization. It will contain an entry for each path that is listed during path canonicalization
    canonicalization_cache: Mutex<LruCache<PathBuf, Vec<(Nfc, OsString)>>>,
}

/// A virtual file.
#[derive(Debug)]
pub struct DirFsFile {
    /// Display info.
    pub file_path: Nfc,
    /// Display info.
    pub dir_path: PathBuf,
    /// File that is open.
    pub file: File,
}

impl DirFs {
    /// Creates a new virtual filesystem.
    pub fn new(path: &Path) -> io::Result<Arc<DirFs>> {
        fs::read_dir(&path)?;
        Ok(Arc::new(DirFs {
            dir_path: path.to_owned(),
            canonicalization_cache: Mutex::new(LruCache::new(CANONICALIZATION_CACHE_SIZE)),
        }))
    }

    /// Maps a path to all candidates that might match the path case insensitively
    ///
    /// The returned paths are already containing the dir path
    fn canonicalize(&self, file_path: &str) -> io::Result<Vec<PathBuf>> {
        let mut candidates = vec![self.dir_path.to_owned()];
        let mut canonicalization_cache = self.canonicalization_cache.lock().map_err(|err| {
            io::Error::new(
                io::ErrorKind::Other,
                format!("DirFs: Error locking canonicalization cache: `{}`", err),
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
                    "special path components are not supported",
                ));
            }
            for candidate in candidates {
                let entries = if let Some(cache_entry) = canonicalization_cache.get(&candidate) {
                    cache_entry
                } else if !candidate.is_dir() {
                    canonicalization_cache.put(candidate.clone(), vec![]);
                    canonicalization_cache
                        .get(&candidate)
                        .expect("we should be able to get a cache key that was just set")
                } else {
                    let entries: io::Result<Vec<_>> = fs::read_dir(&candidate)?
                        .map(|entry_result| {
                            entry_result.and_then(|e| {
                                e.path()
                                    .file_name()
                                    .and_then(|os_string| {
                                        os_string
                                            .to_str()
                                            .map(|s| (Nfc::caseless(s), os_string.to_owned()))
                                    })
                                    .ok_or_else(|| {
                                        io::Error::new(
                                            io::ErrorKind::InvalidInput,
                                            "missing file name when listing files",
                                        )
                                    })
                            })
                        })
                        .collect();
                    let entries = entries?;
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
}

impl VfsLayer for DirFs {
    fn open(&self, file_path: &Nfc) -> io::Result<Box<dyn VfsFile>> {
        let candidates = self.canonicalize(file_path)?;
        if let Some(path) = candidates.iter().find(|x| x.is_file()) {
            Ok(Box::new(DirFsFile {
                file_path: file_path.to_owned(),
                dir_path: self.dir_path.to_owned(),
                file: File::open(&path)?,
            }))
        } else {
            Err(io::ErrorKind::NotFound.into())
        }
    }

    fn read_dir(&self, file_path: &Nfc) -> io::Result<HashSet<Nfc>> {
        let file_path = file_path.trim_end_matches('/');
        let candidates = self.canonicalize(file_path)?;
        let mut result = HashSet::new();

        for candidate in candidates {
            let dir_contents = fs::read_dir(&candidate)?;

            for entry in dir_contents {
                let entry = entry?;
                let file_name_nfc = Nfc::caseless_path(
                    &entry.file_name().to_owned().into_string().map_err(|err| {
                        io::Error::new(
                            io::ErrorKind::InvalidInput,
                            format!(
                                "Could not convert path {:?} to NFC for DirFs: {:?}",
                                entry.file_name(),
                                err
                            ),
                        )
                    })?,
                );
                result.insert(file_name_nfc);
            }
        }

        Ok(result)
    }
}

impl VfsFile for DirFsFile {
    /// Gets the length of the file.
    fn len(&self) -> io::Result<u64> {
        self.file.metadata().map(|x| x.len())
    }
}

impl fmt::Display for DirFs {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "DirFs {{ {:?} }}", self.dir_path)
    }
}

impl fmt::Display for DirFsFile {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "DirFsFile {{ {:?} in {:?} }}",
            self.file_path, self.dir_path
        )
    }
}

impl io::Read for DirFsFile {
    fn read(&mut self, buf: &mut [u8]) -> io::Result<usize> {
        self.file.read(buf)
    }
}

impl io::Seek for DirFsFile {
    fn seek(&mut self, pos: SeekFrom) -> io::Result<u64> {
        self.file.seek(pos)
    }
}

impl io::Write for DirFsFile {
    fn write(&mut self, buf: &[u8]) -> io::Result<usize> {
        self.file.write(buf)
    }
    fn flush(&mut self) -> io::Result<()> {
        self.file.flush()
    }
}
