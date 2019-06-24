//! This module implements a thread safe library database.
//!
//! Rustified version of `src/sgp/LibraryDataBase.cc` and `src/sgp/LibraryDataBase.h`.
//!
//! The mains differences are:
//!  * it is thread safe
//!  * shadowing respects library order and uses the full file path,
//!    the original shadowed whole libraries based on the longest base path

use std::cmp;
use std::collections::HashSet;
use std::fmt;
use std::fs::File;
use std::io::{self, Seek, SeekFrom};
use std::ops;
use std::path::{Component, Path, PathBuf};
use std::sync::{Arc, Mutex, MutexGuard, RwLock};

use crate::file_formats::slf::{SlfEntryState, SlfHeader};

/// Thread safe library database.
#[derive(Debug)]
pub struct LibraryDB {
    inner: Mutex<LibraryDBInner>,
}

/// Library database.
#[derive(Debug)]
pub struct LibraryDBInner {
    /// List of thread safe libraries.
    arc_libraries: Vec<Arc<RwLock<Library>>>,
}

/// Library open for reading.
#[derive(Debug)]
pub struct Library {
    /// Real path of the library file for display purposes.
    library_path: PathBuf,
    /// Library file open for reading.
    library_file: File,
    /// Base path of the entries in the library.
    base_path: String,
    /// List of ok entries in the library.
    entries: Vec<LibraryEntry>,
}

/// File entry in a library open for reading.
#[derive(Debug)]
pub struct LibraryEntry {
    /// File path without the base path of the library.
    file_path: String,
    /// Start of the file data in the library file.
    data_start: u64,
    /// End of the file data in the library file.
    data_end: u64,
}

/// Library file handle that implements Read and Seek.
#[derive(Debug)]
pub struct LibraryFile {
    /// The index of the entry of this file.
    index: usize,
    /// The thread safe library of this file.
    arc_library: Arc<RwLock<Library>>,
    /// Current position in the library file.
    position: u64,
}

impl LibraryDB {
    pub fn new() -> Self {
        Self {
            inner: Mutex::new(LibraryDBInner::new()),
        }
    }

    /// Returns thread safe guard that grants direct access.
    /// While the guard lives, no one else has access to the inner library database.
    pub fn locked(&self) -> MutexGuard<LibraryDBInner> {
        self.inner.lock().unwrap()
    }

    /// Adds a library to the library database.
    pub fn add_library(&mut self, data_dir: &Path, library: &Path) -> io::Result<()> {
        self.locked().add_library(data_dir, library)
    }

    /// Opens a library file for reading.
    pub fn open_file(&self, path: &str) -> io::Result<LibraryFile> {
        self.locked().open_file(path)
    }
}

impl LibraryDBInner {
    fn new() -> Self {
        Self {
            arc_libraries: Vec::new(),
        }
    }

    /// Adds a library to the library database.
    pub fn add_library(&mut self, data_dir: &Path, library: &Path) -> io::Result<()> {
        let library = Library::open_library(&data_dir, &library)?;
        self.arc_libraries.push(Arc::new(RwLock::new(library)));
        return Ok(());
    }

    /// Opens a library file for reading.
    pub fn open_file(&self, path: &str) -> io::Result<LibraryFile> {
        let path = case_insensitive_path(&path);
        for arc_library in &self.arc_libraries {
            let library = arc_library.read().unwrap();
            if let Some(index) = library.find_with_case_insensitive(&path) {
                return Ok(LibraryFile {
                    index,
                    arc_library: arc_library.to_owned(),
                    position: library.entries[index].data_start,
                });
            }
        }
        return Err(io::ErrorKind::NotFound.into());
    }
}

impl Library {
    /// Opens a library for reading.
    fn open_library(data_dir: &Path, library: &Path) -> io::Result<Self> {
        let library_path = find_file(&data_dir, &library)?;
        let mut library_file = File::open(&library_path)?;
        let header = SlfHeader::from_input(&mut library_file)?;
        let base_path = case_insensitive_path(&header.library_path);
        let mut entries: Vec<_> = header
            .entries_from_input(&mut library_file)?
            .iter()
            .filter_map(|slf_entry| match slf_entry.state {
                SlfEntryState::Ok => Some(LibraryEntry {
                    file_path: case_insensitive_path(&slf_entry.file_path),
                    data_start: slf_entry.offset as u64,
                    data_end: slf_entry.offset as u64 + slf_entry.length as u64,
                }),
                _ => None,
            }) // LibraryEntry
            .collect();
        entries.sort();
        let mut unique = HashSet::new();
        for entry in &entries {
            if !unique.insert(&entry.file_path) {
                let path = base_path + &entry.file_path;
                return Err(io::Error::new(
                    io::ErrorKind::InvalidData,
                    format!(
                        "{:?} has multiple entries with path {:?}",
                        library_path, path
                    ),
                ));
            }
        }
        return Ok(Library {
            library_path,
            library_file,
            base_path,
            entries,
        });
    }

    /// Finds the file entry that matches the case-insensitive path.
    /// Returns the entry index or None.
    fn find_with_case_insensitive(&self, path: &str) -> Option<usize> {
        if path.starts_with(&self.base_path) {
            let file_path = path.split_at(self.base_path.len()).1;
            return self
                .entries
                .binary_search_by(|x| x.file_path.as_str().cmp(file_path))
                .ok();
        }
        return None;
    }
}

impl LibraryFile {
    /// Returns the current seek position.
    pub fn current_position(&self) -> u64 {
        let library = self.arc_library.read().unwrap();
        let entry = &library.entries[self.index];
        self.position - entry.data_start
    }

    /// Returns the file size.
    pub fn file_size(&self) -> u64 {
        let library = self.arc_library.read().unwrap();
        let entry = &library.entries[self.index];
        entry.data_end - entry.data_start
    }
}

impl ops::Drop for LibraryDBInner {
    fn drop(&mut self) {
        // make sure there are no pending references.
        for arc_library in &self.arc_libraries {
            let library = arc_library.read().unwrap();
            let n = Arc::strong_count(&arc_library) + Arc::weak_count(&arc_library);
            assert!(n == 1, "{} is shared ({} != 1)", &library, n);
        }
    }
}

impl fmt::Display for Library {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Library {{ {:?} }}", &self.library_path)
    }
}

impl cmp::Ord for LibraryEntry {
    fn cmp(&self, other: &Self) -> cmp::Ordering {
        self.file_path.cmp(&other.file_path)
    }
}

impl cmp::PartialOrd for LibraryEntry {
    fn partial_cmp(&self, other: &Self) -> Option<cmp::Ordering> {
        Some(self.cmp(other))
    }
}

impl cmp::PartialEq for LibraryEntry {
    fn eq(&self, other: &Self) -> bool {
        self.file_path.eq(&other.file_path)
    }
}

impl cmp::Eq for LibraryEntry {}

impl io::Seek for LibraryFile {
    fn seek(&mut self, pos: SeekFrom) -> io::Result<u64> {
        let library = self.arc_library.read().unwrap();
        let entry = &library.entries[self.index];
        let checked_position = match pos {
            SeekFrom::Start(n) => entry.data_start.checked_add(n),
            SeekFrom::Current(n) => checked_add_u64_i64(self.position, n),
            SeekFrom::End(n) => checked_add_u64_i64(entry.data_end, n),
        };
        if let Some(position) = checked_position {
            if position >= entry.data_start {
                self.position = position;
                return Ok(position - entry.data_start);
            }
        }
        // must never become negative or overflow
        return Err(io::ErrorKind::InvalidInput.into());
    }
}

impl io::Read for LibraryFile {
    fn read(&mut self, buf: &mut [u8]) -> io::Result<usize> {
        let mut library = self.arc_library.write().unwrap();
        let end = library.entries[self.index].data_end;
        if self.position >= end {
            return Ok(0);
        }
        let available = end - self.position;
        library.library_file.seek(SeekFrom::Start(self.position))?;
        let read_result = if available < buf.len() as u64 {
            library
                .library_file
                .read(buf.split_at_mut(available as usize).0)
        } else {
            library.library_file.read(buf)
        };
        if let Ok(bytes) = read_result {
            self.position += bytes as u64;
        }
        read_result
    }
}

/// Performs a checked unsigned signed addition.
fn checked_add_u64_i64(a: u64, b: i64) -> Option<u64> {
    if b > 0 {
        a.checked_add(b as u64)
    } else if b < 0 {
        a.checked_sub(-b as u64)
    } else {
        Some(a)
    }
}

/// Gets the case insensitive representation of a path in the library database.
fn case_insensitive_path(path: &str) -> String {
    path.to_ascii_uppercase().replace('\\', "/")
}

/// Finds a filesystem file.
/// dir_path is an absolute path or a path relative to the current directory.
/// file_name is a path relative to dir_path, the normal components are searched case-insensitive (perfect match takes precedence).
fn find_file(dir_path: &Path, file_name: &Path) -> io::Result<PathBuf> {
    let mut path = dir_path.to_owned();
    let components: Vec<_> = file_name.components().collect();
    for (i, component) in components.iter().enumerate() {
        match component {
            Component::Normal(os_str) => {
                if let Some(want) = os_str.to_str() {
                    let found: Vec<_> = path
                        .read_dir()?
                        .filter_map(|x| x.ok()) // DirEntry
                        .filter_map(|x| match x.file_type() {
                            Ok(file_type) => Some((x.file_name(), file_type)),
                            _ => None,
                        }) // (OsString,FileType)
                        .collect();
                    if found.len() > 0 {
                        if found.iter().any(|x| &x.0 == os_str) {
                            path.push(os_str); // perfect match
                            continue;
                        }
                        let want_ascii_uppercase = want.to_ascii_uppercase();
                        let want_dir = i < components.len() - 1;
                        let mut have: Vec<_> = found
                            .iter()
                            .filter(|x| x.1.is_dir() == want_dir)
                            .filter_map(|x| x.0.to_str()) // &str
                            .filter(|x| x.to_ascii_uppercase() == want_ascii_uppercase)
                            .collect();
                        if have.len() > 0 {
                            have.sort();
                            path.push(have[0]); // natural order case insensitive match
                            continue;
                        }
                    }
                    return Err(io::ErrorKind::NotFound.into());
                }
                path.push(component);
            }
            Component::CurDir | Component::ParentDir => path.push(component),
            _ => return Err(io::ErrorKind::InvalidInput.into()),
        }
    }
    if !path.is_file() {
        return Err(io::ErrorKind::NotFound.into());
    }
    return Ok(path);
}
