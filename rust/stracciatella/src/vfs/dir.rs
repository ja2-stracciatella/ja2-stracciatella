//! This module contains a virtual filesystem backed by a SLF file.
#![allow(dead_code)]

use std::fmt;
use std::io;
use std::io::SeekFrom;
use std::path::{Path, PathBuf};

use crate::fs;
use crate::fs::File;
use crate::unicode::Nfc;
use crate::vfs::{VfsFile, VfsLayer};
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
}

impl VfsLayer for DirFs {
    /// Opens a file in the filesystem.
    fn open(&self, file_path: &Nfc) -> io::Result<Box<dyn VfsFile>> {
        let mut candidates = vec![self.dir_path.to_owned()];
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
