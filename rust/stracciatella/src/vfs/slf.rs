//! This module contains a virtual filesystem backed by a SLF file.
#![allow(dead_code)]

use std::convert::TryFrom;
use std::fmt;
use std::io;
use std::io::{Seek, SeekFrom};
use std::path::{Path, PathBuf};
use std::rc::Rc;
use std::sync::{Arc, Mutex};
use std::collections::HashSet;

use crate::file_formats::slf::{SlfEntryState, SlfHeader};
use crate::fs::File;
use crate::math::checked_add_u64_i64;
use crate::unicode::Nfc;
use crate::vfs::{VfsFile, VfsLayer};

/// A read-only case-insensitive virtual filesystem backed by a SLF file.
#[derive(Debug)]
pub struct SlfFs {
    /// Display info.
    pub slf_path: PathBuf,
    /// SLF archive open for reading.
    pub slf_file: Arc<Mutex<File>>,
    /// Case-insensitive base path.
    pub prefix: Nfc,
    /// List of entries
    pub entries: Vec<SlfFsEntry>,
}

/// A file entry.
#[derive(Debug)]
pub struct SlfFsEntry {
    /// Case-insensitive path from the base path.
    pub path: Nfc,
    /// Start of the data.
    pub offset: u32,
    /// Length of the data.
    pub length: u32,
}

/// A virtual file.
#[derive(Debug)]
pub struct SlfFsFile {
    /// Display info.
    pub file_path: Nfc,
    /// Display info.
    pub slf_path: PathBuf,
    /// SLF archive open for reading.
    pub slf_file: Arc<Mutex<File>>,
    /// Start of the data.
    pub offset: u32,
    /// Length of the data.
    pub length: u32,
    /// Current position.
    pub position: u64,
}

impl SlfFs {
    /// Creates a new virtual filesystem.
    pub fn new(path: &Path) -> io::Result<Rc<SlfFs>> {
        let mut slf_file = File::open(&path)?;
        let header = SlfHeader::from_input(&mut slf_file)?;
        let entries: Vec<_> = header
            .entries_from_input(&mut slf_file)?
            .into_iter()
            .filter(|x| x.state == SlfEntryState::Ok)
            .map(|x| SlfFsEntry {
                path: Nfc::caseless_path(&x.file_path),
                offset: x.offset,
                length: x.length,
            })
            .collect();
        Ok(Rc::new(SlfFs {
            slf_path: path.to_owned(),
            slf_file: Arc::new(Mutex::new(slf_file)),
            prefix: Nfc::caseless_path(Nfc::caseless_path(&header.library_path).trim_end_matches('/')),
            entries,
        }))
    }

    /// Canonicalizes to inner path (strips path prefix)
    fn get_slf_path(&self, file_path: &str) -> io::Result<Nfc> {
        if !file_path.starts_with(self.prefix.as_str()) {
            Err(io::ErrorKind::NotFound.into())
        } else {
            let (_, want) = file_path.split_at(self.prefix.len());
            Ok(Nfc::caseless_path(want))
        }
    }
}

impl VfsLayer for SlfFs {
    /// Opens a file in the filesystem.
    fn open(&self, file_path: &Nfc) -> io::Result<Box<dyn VfsFile>> {
        let want = self.get_slf_path(file_path)?;
        let entry_option = self.entries.iter().filter(|x| x.path.as_str() == want.trim_start_matches('/')).nth(0);
        match entry_option {
            Some(entry) => Ok(Box::new(SlfFsFile {
                file_path: file_path.to_owned(),
                slf_path: self.slf_path.to_owned(),
                slf_file: self.slf_file.to_owned(),
                offset: entry.offset,
                length: entry.length,
                position: 0,
            })),
            None => Err(io::ErrorKind::NotFound.into()),
        }
    }

    fn read_dir(&self, file_path: &Nfc) -> io::Result<HashSet<Nfc>> {
        let file_path = file_path.trim_end_matches('/');
        if file_path.len() < self.prefix.len() && self.prefix.starts_with(file_path) {
            // Special case if our prefix is larger than the requested path to list
            // We still need to return parts of the prefix
            let (_, path) = self.prefix.split_at(file_path.len());
            let path = path.trim_start_matches('/');
            if let Some(path) = path.split('/').nth(0) {
                if !path.is_empty() {
                    let mut result = HashSet::new();
                    result.insert(Nfc::caseless_path(path));
                    return Ok(result);
                }
            }
        }
        let want = self.get_slf_path(file_path)?;
        let entries: HashSet<Nfc> = self
            .entries
            .iter()
            .flat_map(|x| {
                if !x.path.starts_with(&want.as_str()) {
                    // This is not part of the listed directory
                    return None;
                }
                // Either a file or subdirectory, so we only return the first path segment
                x.path.split('/').nth(0).map(Nfc::caseless_path)
            })
            .collect();

        if entries.is_empty() {
            Err(io::ErrorKind::NotFound.into())
        } else {
            Ok(entries)
        }
    }
}

impl VfsFile for SlfFsFile {
    /// Gets the length of the file.
    fn len(&self) -> io::Result<u64> {
        Ok(u64::from(self.length))
    }
}

impl fmt::Display for SlfFs {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str("SlfFs { ")?;
        write!(f, "{:?}", self.slf_path)?;
        f.write_str(" }")
    }
}

impl fmt::Display for SlfFsFile {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str("SlfFsFile { ")?;
        write!(f, "{:?} in {:?}", self.file_path, self.slf_path)?;
        f.write_str(" }")
    }
}

impl io::Read for SlfFsFile {
    fn read(&mut self, mut buf: &mut [u8]) -> io::Result<usize> {
        let mut slf_file = self.slf_file.lock().expect("slf_file");
        let available = u64::from(self.length).saturating_sub(self.position);
        if let Ok(available) = usize::try_from(available) {
            if buf.len() > available {
                buf = &mut buf[..available];
            }
        }
        slf_file.seek(SeekFrom::Start(self.position + u64::from(self.offset)))?;
        let read_result = slf_file.read(buf);
        if let Ok(bytes) = read_result {
            self.position += u64::try_from(bytes).expect("u64");
        }
        read_result
    }
}

impl io::Seek for SlfFsFile {
    fn seek(&mut self, pos: SeekFrom) -> io::Result<u64> {
        let position_option = match pos {
            SeekFrom::Start(n) => Some(n),
            SeekFrom::Current(n) => checked_add_u64_i64(self.position, n),
            SeekFrom::End(n) => checked_add_u64_i64(u64::from(self.length), n),
        };
        if let Some(position) = position_option {
            if position.checked_add(u64::from(self.offset)).is_some() {
                self.position = position;
                return Ok(position);
            }
        }
        // underflow or overflow
        Err(io::Error::from(io::ErrorKind::InvalidInput))
    }
}

impl io::Write for SlfFsFile {
    fn write(&mut self, _buf: &[u8]) -> io::Result<usize> {
        Err(io::Error::new(
            io::ErrorKind::PermissionDenied,
            "stracciatella::vfs::slf::SlfFsFile is read-only",
        ))
    }
    fn flush(&mut self) -> io::Result<()> {
        Err(io::Error::new(
            io::ErrorKind::PermissionDenied,
            "stracciatella::vfs::slf::SlfFsFile is read-only",
        ))
    }
}
