//! This module contains a virtual filesystem backed by a SLF file.
#![allow(dead_code)]

use lru::LruCache;
use std::collections::BTreeSet;
use std::ffi::OsString;
use std::fmt;
use std::io;
use std::io::SeekFrom;
use std::num::NonZeroUsize;
use std::path::{Path, PathBuf};
use std::sync::Arc;
use std::sync::Mutex;
use std::time::UNIX_EPOCH;

use crate::fs;
use crate::fs::File;
use crate::unicode::Nfc;
use crate::vfs::{VfsFile, VfsLayer, VfsMetadata, VfsOpenOptions, read_only_error};

/// The size of the cache used for canonicalization
const CANONICALIZATION_CACHE_SIZE: usize = 256;

/// A case-insensitive virtual filesystem backed by a filesystem directory.
#[derive(Debug)]
pub struct DirFs {
    /// Path to the directory.
    pub dir_path: PathBuf,
    /// Whether this layer accepts writes.
    ///
    /// This is intent, not capability: the vanilla game data and the mod directories are mounted
    /// read-only even though the process could write to them.
    writable: bool,
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
    /// Creates a new read-only virtual filesystem.
    pub fn new(path: &Path) -> io::Result<Arc<DirFs>> {
        Self::with_mode(path, false)
    }

    /// Creates a new virtual filesystem that accepts writes.
    pub fn new_writable(path: &Path) -> io::Result<Arc<DirFs>> {
        Self::with_mode(path, true)
    }

    /// Creates a new virtual filesystem.
    pub fn with_mode(path: &Path, writable: bool) -> io::Result<Arc<DirFs>> {
        fs::read_dir(path)?;
        Ok(Arc::new(DirFs {
            dir_path: path.to_owned(),
            writable,
            canonicalization_cache: Mutex::new(LruCache::new(
                NonZeroUsize::new(CANONICALIZATION_CACHE_SIZE).unwrap(),
            )),
        }))
    }

    /// Maps a path to a concrete filesystem path, whether or not it exists yet.
    ///
    /// Existing components are matched case insensitively, components that do not exist yet are
    /// used verbatim, so this also works for the target of a create or a rename.
    fn to_fs_path(&self, file_path: &str) -> io::Result<PathBuf> {
        for component in file_path.split('/') {
            if component == "." || component == ".." {
                return Err(io::Error::new(
                    io::ErrorKind::InvalidInput,
                    "special path components are not supported",
                ));
            }
        }
        Ok(fs::resolve_existing_components(
            Path::new(file_path),
            Some(&self.dir_path),
            true,
        ))
    }

    /// Fails unless this layer accepts writes.
    fn ensure_writable(&self) -> io::Result<()> {
        if self.writable {
            Ok(())
        } else {
            Err(read_only_error(self))
        }
    }

    /// Drops the cached listings of every directory above `path`.
    ///
    /// Needed after a write, because those listings are now stale. Creating a directory can add
    /// more than one level at a time, so stopping at the direct parent is not enough.
    fn invalidate_cache_for(&self, path: &Path) {
        let Ok(mut cache) = self.canonicalization_cache.lock() else {
            return;
        };
        let mut current = path.parent();
        while let Some(dir) = current {
            cache.pop(dir);
            if dir == self.dir_path {
                break;
            }
            current = dir.parent();
        }
    }

    /// Maps a path to all candidates that might match the path case insensitively
    ///
    /// The returned paths are already containing the dir path.
    /// The case of `file_path` does not matter, it is folded here.
    fn canonicalize(&self, file_path: &str) -> io::Result<Vec<PathBuf>> {
        let mut candidates = vec![self.dir_path.to_owned()];
        let mut canonicalization_cache = self.canonicalization_cache.lock().map_err(|err| {
            io::Error::other(format!(
                "DirFs: Error locking canonicalization cache: `{}`",
                err
            ))
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
            let want = Nfc::caseless(want);
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
                    if &want == nfc {
                        next.push(candidate.join(os_string));
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
                file: File::open(path)?,
            }))
        } else {
            Err(io::ErrorKind::NotFound.into())
        }
    }

    fn exists(&self, file_path: &Nfc) -> io::Result<bool> {
        let candidates = self.canonicalize(file_path)?;
        Ok(!candidates.is_empty())
    }

    fn read_dir(&self, file_path: &Nfc) -> io::Result<BTreeSet<Nfc>> {
        let file_path = file_path.trim_end_matches('/');
        let candidates = self.canonicalize(file_path)?;
        let mut result = BTreeSet::new();

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

    fn is_writable(&self) -> bool {
        self.writable
    }

    fn metadata(&self, file_path: &Nfc) -> io::Result<VfsMetadata> {
        let candidates = self.canonicalize(file_path)?;
        let path = candidates.first().ok_or(io::ErrorKind::NotFound)?;
        let metadata = fs::metadata(path)?;
        let is_dir = metadata.is_dir();
        Ok(VfsMetadata {
            is_dir,
            len: if is_dir { 0 } else { metadata.len() },
            modified_secs: metadata
                .modified()
                .ok()
                .and_then(|time| time.duration_since(UNIX_EPOCH).ok())
                .map(|duration| duration.as_secs_f64()),
            read_only: metadata.permissions().readonly(),
        })
    }

    fn resolve_existing_components(&self, file_path: &Nfc) -> Nfc {
        let resolved = match self.to_fs_path(file_path) {
            Ok(path) => path,
            Err(_) => return file_path.to_owned(),
        };
        // Keeps the case of the resolved components, which is the whole point of resolving them.
        resolved
            .strip_prefix(&self.dir_path)
            .ok()
            .and_then(|path| path.to_str())
            .map(|path| Nfc::path(&path.replace('\\', "/")))
            .unwrap_or_else(|| file_path.to_owned())
    }

    fn open_with_options(
        &self,
        file_path: &Nfc,
        options: VfsOpenOptions,
    ) -> io::Result<Box<dyn VfsFile>> {
        if !options.is_write() {
            return self.open(file_path);
        }
        self.ensure_writable()?;
        let path = self.to_fs_path(file_path)?;
        let file = fs::OpenOptions::new()
            .read(options.read)
            .write(options.write)
            .append(options.append)
            .truncate(options.truncate)
            .create(options.create)
            .create_new(options.create_new)
            .open(&path)?;
        self.invalidate_cache_for(&path);
        Ok(Box::new(DirFsFile {
            file_path: file_path.to_owned(),
            dir_path: self.dir_path.to_owned(),
            file,
        }))
    }

    fn remove_file(&self, file_path: &Nfc) -> io::Result<()> {
        self.ensure_writable()?;
        let candidates = self.canonicalize(file_path)?;
        let path = candidates
            .into_iter()
            .find(|candidate| candidate.is_file())
            .ok_or(io::ErrorKind::NotFound)?;
        fs::remove_file(&path)?;
        self.invalidate_cache_for(&path);
        Ok(())
    }

    fn create_dir(&self, file_path: &Nfc) -> io::Result<()> {
        self.ensure_writable()?;
        let path = self.to_fs_path(file_path)?;
        if path.is_dir() {
            return Ok(());
        }
        fs::create_dir_all(&path)?;
        self.invalidate_cache_for(&path);
        Ok(())
    }

    fn rename(&self, from: &Nfc, to: &Nfc) -> io::Result<()> {
        self.ensure_writable()?;
        let from_path = self
            .canonicalize(from)?
            .into_iter()
            .next()
            .ok_or(io::ErrorKind::NotFound)?;
        let to_path = self.to_fs_path(to)?;
        fs::rename(from_path.clone(), to_path.clone())?;
        self.invalidate_cache_for(&from_path);
        self.invalidate_cache_for(&to_path);
        Ok(())
    }

    fn free_space(&self) -> io::Result<u64> {
        fs::free_space(&self.dir_path)
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
