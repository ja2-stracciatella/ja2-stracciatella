//! This module implements virtual filesystems.
//!
//! The paths are case insensitive.
//! It does not support path components `.` and `..`.
#![allow(dead_code)]

pub mod dir;
pub mod slf;

use std::fmt;
use std::io;
use std::io::SeekFrom;
use std::path::Path;

use crate::unicode::Nfc;
use crate::vfs::dir::{DirFs, DirFsFile};
use crate::vfs::slf::{SlfFs, SlfFsFile};

/// A virtual filesystem that mounts other filesystems.
#[derive(Debug, Default)]
pub struct Vfs {
    /// List of entries.
    pub entries: Vec<VfsEntry>,
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

impl Vfs {
    /// Creates a new virtual filesystem.
    pub fn new() -> Vfs {
        Vfs::default()
    }

    /// Adds an overlay filesystem backed by a filesystem directory.
    pub fn add_dir(&mut self, order: i32, path: &Path) -> io::Result<()> {
        self.insert_entry(VfsEntry {
            order,
            source: VfsSource::Dir(DirFs::new(&path)?),
        });
        Ok(())
    }

    /// Adds an overlay filesystem backed by a SLF file.
    pub fn add_slf(&mut self, order: i32, path: &Path) -> io::Result<()> {
        self.insert_entry(VfsEntry {
            order,
            source: VfsSource::Slf(SlfFs::new(&path)?),
        });
        Ok(())
    }

    /// Opens a file.
    pub fn open(&self, file_path: &Nfc) -> io::Result<VfsFile> {
        for entry in self.entries.iter() {
            let file_result = match &entry.source {
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

    /// Inserts an entry in the position according to the order.
    fn insert_entry(&mut self, entry: VfsEntry) {
        let index = self
            .entries
            .iter()
            .position(|x| x.order > entry.order)
            .unwrap_or_else(|| self.entries.len());
        self.entries.insert(index, entry);
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
            write!(f, "{}, ", entry.source)?;
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
