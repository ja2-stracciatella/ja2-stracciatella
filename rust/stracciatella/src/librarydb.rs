//! This module implements a thread safe library database.
//!
//! A library database contains a collection of libraries.
//! A library contains a collection of file entries.
//! Libraries that contain multiple ok entries with the same case-insensitive file path are rejected.
//!
//! The order of the libraries matters.
//! When a file path matches multiple entries, the first entry shadows the other entries.
//!
//!
//! # Rustified
//!
//! This module is a rustified version of `src/sgp/LibraryDataBase.cc` and `src/sgp/LibraryDataBase.h`.
//!
//! The main differences are:
//!  * it is thread safe
//!  * shadowing respects library order and uses the full file path,
//!    the original shadowed libraries based on the longest base path
//!
//!
//! # FFI
//!
//! [`stracciatella_c_api::c::librarydb`] contains a C interface for this module.
//!
//! [`stracciatella_c_api::c::librarydb`]: ../../stracciatella_c_api/c/librarydb/index.html

use std::cmp;
use std::collections::HashSet;
use std::fmt;
use std::fs::File;
use std::io;
use std::io::{Seek, SeekFrom};
use std::ops;
use std::path::{Component, Path, PathBuf};
use std::sync::{Arc, Mutex, MutexGuard, RwLock};

use crate::file_formats::slf::{SlfEntryState, SlfHeader};
use crate::math::checked_add_u64_i64;
use crate::unicode::Nfc;

/// Thread safe library database.
#[derive(Debug)]
pub struct LibraryDB {
    inner: Mutex<LibraryDBInner>,
}

/// Library database.
#[derive(Debug)]
pub struct LibraryDBInner {
    /// Thread safe libraries.
    arc_libraries: Vec<Arc<RwLock<Library>>>,
}

/// Library.
/// Keeps the library file open for reading and a list of entries that are ok.
/// The library file contains the data of each entry.
#[derive(Debug)]
pub struct Library {
    /// Real path of the library file for display purposes.
    library_path: PathBuf,
    /// Library file open for reading.
    library_file: File,
    /// Caseless base path of the entries in the library.
    base_path: Nfc,
    /// List of ok entries in the library.
    entries: Vec<LibraryEntry>,
}

/// Library entry.
/// Represents a file in a library.
#[derive(Debug)]
pub struct LibraryEntry {
    /// Caseless file path without the base path of the library.
    file_path: Nfc,
    /// Start of the file data in the library file.
    data_start: u64,
    /// End of the file data in the library file.
    data_end: u64,
}

/// Library file.
/// Provides access to the data of a library entry through Read and Seek.
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
    /// Constructor.
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

    /// Opens and adds a library at the end of library database.
    pub fn add_library(&mut self, data_dir: &Path, library: &Path) -> io::Result<()> {
        self.locked().add_library(data_dir, library)
    }

    /// Opens a library file for reading.
    /// The file must be dropped before the library database is dropped.
    pub fn open_file(&self, path: &str) -> io::Result<LibraryFile> {
        self.locked().open_file(path)
    }
}

impl LibraryDBInner {
    /// Constructor.
    fn new() -> Self {
        Self {
            arc_libraries: Vec::new(),
        }
    }

    /// Opens and adds a library at the end of library database.
    pub fn add_library(&mut self, data_dir: &Path, library: &Path) -> io::Result<()> {
        let library = Library::open(&data_dir, &library)?;
        self.arc_libraries.push(Arc::new(RwLock::new(library)));
        Ok(())
    }

    /// Opens a library file for reading.
    /// The file must be dropped before the library database is dropped.
    pub fn open_file(&self, path: &str) -> io::Result<LibraryFile> {
        let path = Nfc::caseless_path(&path);
        for arc_library in &self.arc_libraries {
            let library = arc_library.read().unwrap();
            if let Some(index) = library.find(&path) {
                return Ok(LibraryFile {
                    index,
                    arc_library: arc_library.to_owned(),
                    position: library.entries[index].data_start,
                });
            }
        }
        Err(io::ErrorKind::NotFound.into())
    }
}

impl Library {
    /// Opens a library for reading.
    fn open(data_dir: &Path, library: &Path) -> io::Result<Self> {
        let library_path = find_file(&data_dir, &library)?;
        let mut library_file = File::open(&library_path)?;
        let header = SlfHeader::from_input(&mut library_file)?;
        let base_path = Nfc::caseless_path(&header.library_path);
        let mut entries: Vec<_> = header
            .entries_from_input(&mut library_file)?
            .iter()
            .filter_map(|slf_entry| match slf_entry.state {
                SlfEntryState::Ok => Some(LibraryEntry {
                    file_path: Nfc::caseless_path(&slf_entry.file_path),
                    data_start: u64::from(slf_entry.offset),
                    data_end: u64::from(slf_entry.offset) + u64::from(slf_entry.length),
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
        Ok(Library {
            library_path,
            library_file,
            base_path,
            entries,
        })
    }

    /// Finds the file entry identified by path.
    /// Returns the entry index or None.
    /// Expects path to be a caseless path.
    /// Expects the entries to be sorted.
    fn find(&self, path: &Nfc) -> Option<usize> {
        if path.starts_with(&self.base_path.as_str()) {
            let file_path = path.split_at(self.base_path.len()).1;
            return self
                .entries
                .binary_search_by(|x| x.file_path.as_str().cmp(file_path))
                .ok();
        }
        None
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

impl Default for LibraryDB {
    fn default() -> Self {
        Self::new()
    }
}

/// Panics if a library file is still open during the destruction of LibraryDBInner.
impl ops::Drop for LibraryDBInner {
    fn drop(&mut self) {
        for arc_library in &self.arc_libraries {
            let library = arc_library.read().unwrap();
            let n = Arc::strong_count(&arc_library) + Arc::weak_count(&arc_library);
            assert!(n == 1, "{} is being shared ({} != 1)", &library, n);
        }
    }
}

/// Library only displays the path by default.
impl fmt::Display for Library {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Library {{ {:?} }}", &self.library_path)
    }
}

/// Library entries are ordered by file path.
impl cmp::Ord for LibraryEntry {
    fn cmp(&self, other: &Self) -> cmp::Ordering {
        self.file_path.cmp(&other.file_path)
    }
}

/// Library entries are ordered by file path.
impl cmp::PartialOrd for LibraryEntry {
    fn partial_cmp(&self, other: &Self) -> Option<cmp::Ordering> {
        Some(self.cmp(other))
    }
}

/// Library entries are ordered by file path.
impl cmp::PartialEq for LibraryEntry {
    fn eq(&self, other: &Self) -> bool {
        self.file_path.eq(&other.file_path)
    }
}

impl cmp::Eq for LibraryEntry {}

/// LibraryFile displays the library and the file path by default.
impl fmt::Display for LibraryFile {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let library = self.arc_library.read().unwrap();
        let entry = &library.entries[self.index];
        write!(f, "LibraryFile {{ {}, {:?} }}", &library, &entry.file_path)
    }
}

/// LibraryFile seeks the data of a library entry.
impl io::Seek for LibraryFile {
    fn seek(&mut self, pos: SeekFrom) -> io::Result<u64> {
        let library = self.arc_library.write().unwrap();
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
        Err(io::ErrorKind::InvalidInput.into())
    }
}

/// LibraryFile reads the data of a library entry.
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

/// Finds a filesystem file.
/// dir_path is an absolute path or a path relative to the current directory.
/// file_name is a path relative to dir_path, the normal components are searched case-insensitive (perfect match takes precedence).
/// file_name cannot exit dir_path
fn find_file(dir_path: &Path, file_name: &Path) -> io::Result<PathBuf> {
    let mut path = dir_path.to_owned();
    let components: Vec<_> = file_name.components().collect();
    let mut depth = 0;
    for (i, component) in components.iter().enumerate() {
        match component {
            Component::Normal(os_str) => {
                depth += 1;
                if let Some(want_caseless) = os_str.to_str().map(|x| Nfc::caseless(x)) {
                    let found: Vec<_> = path
                        .read_dir()?
                        .filter_map(|x| x.ok()) // DirEntry
                        .filter_map(|x| match x.file_type() {
                            Ok(file_type) => Some((x.file_name(), file_type)),
                            _ => None,
                        }) // (OsString,FileType)
                        .collect();
                    if !found.is_empty() {
                        if found.iter().any(|x| &x.0 == os_str) {
                            path.push(os_str); // perfect match
                            continue;
                        }
                        let want_dir = i < components.len() - 1;
                        let mut have: Vec<_> = found
                            .iter()
                            .filter(|x| x.1.is_dir() == want_dir)
                            .filter_map(|x| x.0.to_str()) // &str
                            .filter(|x| Nfc::caseless(x) == want_caseless)
                            .collect();
                        if !have.is_empty() {
                            have.sort();
                            path.push(have[0]); // natural order caseless match
                            continue;
                        }
                    }
                    return Err(io::ErrorKind::NotFound.into());
                }
                path.push(component);
            }
            Component::CurDir => {}
            Component::ParentDir => {
                if depth == 0 {
                    return Err(io::ErrorKind::InvalidInput.into());
                }
                depth -= 1;
                path.pop();
            }
            _ => return Err(io::ErrorKind::InvalidInput.into()),
        }
    }
    if !path.is_file() {
        return Err(io::ErrorKind::NotFound.into());
    }
    Ok(path)
}

#[cfg(test)]
pub mod tests {
    use std::fs::OpenOptions;
    use std::io::{Read, Seek, SeekFrom, Write};
    use std::path::{Path, PathBuf};

    use tempfile::TempDir;

    use crate::file_formats::slf::{SlfEntry, SlfEntryState, SlfHeader};
    use crate::librarydb::*;

    fn make_slf(dir: &Path, name: &str, library_path: &str, entry_paths: &[&str]) -> PathBuf {
        let header = SlfHeader {
            library_name: name.to_owned(),
            library_path: library_path.to_owned(),
            num_entries: entry_paths.len() as i32,
            ok_entries: entry_paths.len() as i32,
            sort: 0xFFFF,
            version: 0x200,
            contains_subdirectories: if library_path.is_empty() { 0 } else { 1 },
        };
        let path = dir.join(&name);
        let mut file = OpenOptions::new()
            .write(true)
            .create_new(true)
            .open(&path)
            .unwrap();
        header.to_output(&mut file).unwrap();
        let mut entries = entry_paths
            .iter()
            .map(|&entry_path| {
                let offset = file.seek(SeekFrom::Current(0)).unwrap();
                let data = name.as_bytes();
                file.write_all(&data).unwrap();
                SlfEntry {
                    file_path: entry_path.to_owned(),
                    offset: offset as u32,
                    length: data.len() as u32,
                    state: SlfEntryState::Ok,
                    file_time: 0,
                }
            })
            .collect::<Vec<SlfEntry>>();
        entries.sort_by(|a, b| a.file_path.cmp(&b.file_path));
        header.entries_to_output(&mut file, &entries).unwrap();
        file.sync_all().unwrap();
        path
    }

    fn library_file_data(ldb: &LibraryDB, path: &str) -> Vec<u8> {
        let mut file = ldb.open_file(&path).unwrap();
        let mut data = Vec::new();
        file.read_to_end(&mut data).unwrap();
        data
    }

    /// Creates a temporary data dir that is removed when TempDir is closed or goes out of scope.
    pub fn data_dir() -> (TempDir, PathBuf) {
        let tmp = TempDir::new().unwrap();
        let dir = tmp.path().to_owned();
        // data.slf contains foo.txt
        let name = "data.slf";
        let library_path = "";
        let entry_paths = ["foo.txt"];
        make_slf(&dir, &name, &library_path, &entry_paths);
        // foo.slf contains foo/bar.txt and foo/bar/baz.txt
        let name = "foo.slf";
        let library_path = "foo\\";
        let entry_paths = ["bar.txt", "bar\\baz.txt"];
        make_slf(&dir, &name, &library_path, &entry_paths);
        // foobar.slf contains foo/bar/baz.txt
        let name = "foobar.slf";
        let library_path = "foo\\bar\\";
        let entry_paths = ["baz.txt"];
        make_slf(&dir, &name, &library_path, &entry_paths);
        (tmp, dir)
    }

    #[test]
    fn open_and_read() {
        let (tmp, dir) = data_dir();

        let mut ldb = LibraryDB::new();
        ldb.add_library(&dir, Path::new("data.slf")).unwrap();
        let data = library_file_data(&ldb, "foo.txt");
        assert_eq!(&data, b"data.slf");

        tmp.close().unwrap();
    }

    #[test]
    fn open_and_read_case_insensitive() {
        let (tmp, dir) = data_dir();

        let mut ldb = LibraryDB::new();
        ldb.add_library(&dir, Path::new("DATA.slf")).unwrap();
        let data = library_file_data(&ldb, "FOO.txt");
        assert_eq!(&data, b"data.slf");

        tmp.close().unwrap();
    }

    #[test]
    fn seek() {
        let (tmp, dir) = data_dir();

        let mut ldb = LibraryDB::new();
        ldb.add_library(&dir, Path::new("data.slf")).unwrap();
        let mut file = ldb.open_file("foo.txt").unwrap();

        assert_eq!(file.seek(SeekFrom::End(0)).unwrap(), 8);
        assert_eq!(file.seek(SeekFrom::Current(4)).unwrap(), 12);
        assert_eq!(file.seek(SeekFrom::Current(-8)).unwrap(), 4);
        assert!(file.seek(SeekFrom::Current(-5)).is_err());
        assert_eq!(file.seek(SeekFrom::Start(0)).unwrap(), 0);

        tmp.close().unwrap();
    }

    #[test]
    fn library_order_matters() {
        let (tmp, dir) = data_dir();

        let mut ldb = LibraryDB::new();
        ldb.add_library(&dir, Path::new("foo.slf")).unwrap();
        ldb.add_library(&dir, Path::new("foobar.slf")).unwrap();
        let data = library_file_data(&ldb, "foo/bar/baz.txt");
        assert_eq!(&data, b"foo.slf");

        let mut ldb = LibraryDB::new();
        ldb.add_library(&dir, Path::new("foobar.slf")).unwrap();
        ldb.add_library(&dir, Path::new("foo.slf")).unwrap();
        let data = library_file_data(&ldb, "foo/bar/baz.txt");
        assert_eq!(&data, b"foobar.slf");

        tmp.close().unwrap();
    }

    #[test]
    fn case_insensitive_file_paths() {
        let (tmp, dir) = data_dir();

        let mut ldb = LibraryDB::new();
        ldb.add_library(&dir, &Path::new("foo.slf")).unwrap();
        let data = library_file_data(&ldb, "FOO/bar.txt");
        assert_eq!(&data, b"foo.slf");
        let data = library_file_data(&ldb, "foo/BAR.TXT");
        assert_eq!(&data, b"foo.slf");
        let data = library_file_data(&ldb, "foo/BAR/baz.txt");
        assert_eq!(&data, b"foo.slf");
        let data = library_file_data(&ldb, "foo/bar/BAZ.TXT");
        assert_eq!(&data, b"foo.slf");

        tmp.close().unwrap();
    }

    #[test]
    fn allow_both_seprators() {
        let (tmp, dir) = data_dir();

        let mut ldb = LibraryDB::new();
        ldb.add_library(&dir, &Path::new("foo.slf")).unwrap();
        let data = library_file_data(&ldb, "foo/bar/baz.txt");
        assert_eq!(&data, b"foo.slf");
        let data = library_file_data(&ldb, "foo/bar\\baz.txt");
        assert_eq!(&data, b"foo.slf");
        let data = library_file_data(&ldb, "foo\\bar\\baz.txt");
        assert_eq!(&data, b"foo.slf");
        let data = library_file_data(&ldb, "foo\\bar/baz.txt");
        assert_eq!(&data, b"foo.slf");

        tmp.close().unwrap();
    }
}
