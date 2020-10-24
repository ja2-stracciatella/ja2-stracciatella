//! This module contains a virtual filesystem backed by a specific directory in the file system.
//!
//! All paths opened in this VFS are relative to the base directory passed in `new`. They should
//! not start with a `/`.
//!
//! ## Assumptions
//!
//! There are some assumptions we make about the directory in oder to make everything work smoothly.
//!
//! - The directory and file structure within the directory does not change during a single run of the game
//! - All paths within the directory can be de- and encoded to UTF-8
//!
#![allow(dead_code)]

use std::fmt;
use std::io;
use std::io::SeekFrom;
use std::path::{Path, PathBuf};

use crate::fs;
use crate::fs::File;
use crate::unicode::Nfc;
use crate::vfs::{VfsFile, VfsLayer};
use std::collections::HashSet;
use std::rc::Rc;

/// A case-insensitive virtual filesystem backed by a filesystem directory.
#[derive(Debug)]
pub struct DirFs {
    /// Path to the directory.
    pub dir_path: PathBuf,
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
    pub fn new(path: &Path) -> io::Result<Rc<DirFs>> {
        fs::read_dir(&path)?;
        Ok(Rc::new(DirFs {
            dir_path: path.to_owned(),
        }))
    }

    /// Maps a path to all candidates that might match the path case insensitively
    ///
    /// The returned paths are already containing the dir path
    fn canonicalize(&self, file_path: &str) -> io::Result<Vec<PathBuf>> {
        let mut candidates = vec![self.dir_path.to_owned()];

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
                if !candidate.is_dir() {
                    continue;
                }
                for entry_result in fs::read_dir(&candidate)? {
                    let path = entry_result?.path();
                    let is_match = path
                        .file_name()
                        .and_then(|x| x.to_str())
                        .map(|x| want == Nfc::caseless(x).as_str())
                        .unwrap_or(false);
                    if is_match {
                        next.push(path);
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
        if let Some(path) = candidates.iter().filter(|x| x.is_file()).nth(0) {
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
