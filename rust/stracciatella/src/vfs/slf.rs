//! This module contains a virtual filesystem backed by a SLF File.
//!
//! The paths that are contained in this filesystem will be prefixed with the SLF files library path (e.g. `interface` for `Interface.slf`)
//!
//! ## Assumptions
//!
//! There are some assumptions we make about the AssetManager in oder to make everything work smoothly.
//!
//! - The SLF file does not change during a single run of the game
//! - All paths within the SLF file can be de- and encoded to UTF-8
//!
#![allow(dead_code)]

use std::collections::HashMap;
use std::collections::HashSet;
use std::convert::TryFrom;
use std::fmt;
use std::io;
use std::io::{Seek, SeekFrom};
use std::rc::Rc;
use std::sync::{Arc, Mutex};

use crate::file_formats::slf::{SlfEntryState, SlfHeader};
use crate::math::checked_add_u64_i64;
use crate::unicode::Nfc;
use crate::vfs::{VfsFile, VfsLayer};

/// A read-only case-insensitive virtual filesystem backed by a SLF file.
#[derive(Debug)]
pub struct SlfFs {
    /// Display info.
    pub slf_path: String,
    /// SLF archive open for reading.
    pub slf_file: Arc<Mutex<Box<dyn VfsFile>>>,
    /// Case-insensitive base path.
    pub prefix: Nfc,
    /// List of entries
    pub entries: HashMap<Nfc, SlfFsEntry>,
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
    pub slf_path: String,
    /// SLF archive open for reading.
    pub slf_file: Arc<Mutex<Box<dyn VfsFile>>>,
    /// Start of the data.
    pub offset: u32,
    /// Length of the data.
    pub length: u32,
    /// Current position.
    pub position: u64,
}

impl SlfFs {
    /// Creates a new virtual filesystem.
    pub fn new(mut slf_file: Box<dyn VfsFile>) -> io::Result<Rc<SlfFs>> {
        let header = SlfHeader::from_input(&mut slf_file)?;
        let prefix = Nfc::caseless_path(&header.library_path.trim_end_matches('/'));
        let entries: HashMap<_, _> = header
            .entries_from_input(&mut slf_file)?
            .into_iter()
            .filter(|x| x.state == SlfEntryState::Ok)
            .map(|x| {
                let path = Nfc::caseless_path(&x.file_path.trim_start_matches('/'));
                let full_path = prefix.clone() + &path;
                let entry = SlfFsEntry {
                    path,
                    offset: x.offset,
                    length: x.length,
                };
                (full_path, entry)
            })
            .collect();
        Ok(Rc::new(SlfFs {
            slf_path: format!("{}", slf_file),
            slf_file: Arc::new(Mutex::new(slf_file)),
            prefix,
            entries,
        }))
    }
}

impl VfsLayer for SlfFs {
    /// Opens a file in the filesystem.
    fn open(&self, file_path: &Nfc) -> io::Result<Box<dyn VfsFile>> {
        match self.entries.get(file_path) {
            Some(entry) => Ok(Box::new(SlfFsFile {
                file_path: file_path.to_owned(),
                slf_path: self.slf_path.to_owned(),
                slf_file: self.slf_file.clone(),
                offset: entry.offset,
                length: entry.length,
                position: 0,
            })),
            None => Err(io::ErrorKind::NotFound.into()),
        }
    }

    fn read_dir(&self, path: &Nfc) -> io::Result<HashSet<Nfc>> {
        // Remove trailing slashes from directories
        let path = path.trim_end_matches('/');
        let entries: HashSet<Nfc> = self
            .entries
            .keys()
            .flat_map(|x| {
                if path.is_empty() {
                    // If the root path is requested, just return the first path segment
                    x.split('/').nth(0).map(Nfc::from)
                } else {
                    if !x.starts_with(&path) {
                        // This is not part of the listed directory
                        return None;
                    }
                    // Either a file or subdirectory, so we only return the first path segment
                    // As the entries already use Nfc::caseless_path, we dont need to use Nfc::caseless_path again
                    x.split_at(path.len()).1.split('/').nth(1).map(Nfc::from)
                }
            })
            .collect();

        // As we dont really have directories in the SLF file, if we have no
        // matching paths, we assume the directory does not exist
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
        f.write_str("SlfFs { source: ")?;
        write!(f, "{}", self.slf_path)?;
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
