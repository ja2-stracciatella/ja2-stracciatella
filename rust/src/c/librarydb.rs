//! This module contains the C interface for [`stracciatella::librarydb`].
//!
//! [`stracciatella::librarydb`]: ../../librarydb/index.html

use std::io;
use std::io::{Read, Seek, SeekFrom};

use crate::c::common::*;
use crate::librarydb::{LibraryDB, LibraryFile};

/// Constructor.
/// The caller is responsible for the memory of the library database.
#[no_mangle]
pub extern "C" fn LibraryDB_create() -> *mut LibraryDB {
    into_ptr(LibraryDB::new())
}

/// Destructor.
/// The caller is no longer responsible for the memory of the library database.
#[no_mangle]
pub extern "C" fn LibraryDB_destroy(ldb: *mut LibraryDB) {
    let _drop_me = from_ptr(ldb);
}

/// Adds a library to the end of the library database.
/// Returns true is successful, false otherwise.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn LibraryDB_push(
    ldb: *mut LibraryDB,
    data_dir: *const c_char,
    library: *const c_char,
) -> bool {
    let ldb = unsafe_mut(ldb);
    let data_dir = path_from_c_str_or_panic(unsafe_c_str(data_dir));
    let library = path_from_c_str_or_panic(unsafe_c_str(library));
    match ldb.add_library(data_dir, library) {
        Err(err) => {
            remember_rust_error(format!("{:?}", err));
            false
        }
        Ok(_) => {
            forget_rust_error();
            true
        }
    }
}

/// Opens a library database file for reading.
/// Returns the file on success and null on error.
/// The caller is responsible for the library file memory.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn LibraryFile_open(ldb: *mut LibraryDB, path: *const c_char) -> *mut LibraryFile {
    let ldb = unsafe_mut(ldb);
    let path = str_from_c_str_or_panic(unsafe_c_str(path));
    match ldb.open_file(&path) {
        Err(err) => {
            remember_rust_error(format!("{:?}", err));
            std::ptr::null_mut()
        }
        Ok(file) => {
            forget_rust_error();
            into_ptr(file)
        }
    }
}

/// Closes a library database file.
/// The caller is no longer responsible for the library file memory.
#[no_mangle]
pub extern "C" fn LibraryFile_Close(file: *mut LibraryFile) {
    let _drop_me = from_ptr(file);
}

/// Seeks a library database file.
/// `from` expects FileSeekMode values (see src/sgp/SGPFile.h)
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn LibraryFile_Seek(file: *mut LibraryFile, distance: i64, from: c_int) -> bool {
    let file = unsafe_mut(file);
    let seek_result = match from {
        0 if distance >= 0 => file.seek(SeekFrom::Start(distance as u64)),
        1 => file.seek(SeekFrom::End(distance)),
        2 => file.seek(SeekFrom::Current(distance)),
        _ => Err(io::ErrorKind::InvalidInput.into()),
    };
    match seek_result {
        Err(err) => {
            remember_rust_error(format!("{:?}", err));
            false
        }
        Ok(_) => {
            forget_rust_error();
            true
        }
    }
}

/// Reads from a library database file.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn LibraryFile_Read(
    file: *mut LibraryFile,
    buffer: *mut u8,
    buffer_length: size_t,
) -> bool {
    let file = unsafe_mut(file);
    let buffer = unsafe_slice_mut(buffer, buffer_length);
    match file.read_exact(buffer) {
        Err(err) => {
            remember_rust_error(format!("{:?}", err));
            false
        }
        Ok(_) => {
            forget_rust_error();
            true
        }
    }
}

/// Gets the current position in a library database file.
#[no_mangle]
pub extern "C" fn LibraryFile_GetPos(file: *mut LibraryFile) -> u64 {
    let file = unsafe_mut(file);
    file.current_position()
}

/// Gets the size of a library database file.
#[no_mangle]
pub extern "C" fn LibraryFile_GetSize(file: *mut LibraryFile) -> u64 {
    let file = unsafe_mut(file);
    file.file_size()
}

#[cfg(test)]
mod tests {
    use crate::c::common::*;
    use crate::c::librarydb::*;

    // FileSeekMode values (see src/sgp/SGPFile.h)
    const FILE_SEEK_FROM_START: c_int = 0;
    const FILE_SEEK_FROM_END: c_int = 1;
    const FILE_SEEK_FROM_CURRENT: c_int = 2;

    fn read_to_end(c_file: *mut LibraryFile) -> Vec<u8> {
        let size = LibraryFile_GetSize(c_file) as usize;
        let pos = LibraryFile_GetPos(c_file) as usize;
        let mut data = vec![0u8; size - pos];
        assert!(LibraryFile_Read(c_file, data.as_mut_ptr(), size - pos));
        data
    }

    fn library_file_data(c_ldb: *mut LibraryDB, path: &str) -> Vec<u8> {
        let c_path = c_string_from_str(&path);
        let c_file = LibraryFile_open(c_ldb, c_path.as_ptr()); // must manage the memory
        assert_ne!(c_file, std::ptr::null_mut());
        assert_eq!(LibraryFile_GetPos(c_file), 0);
        let data = read_to_end(c_file);
        LibraryFile_Close(c_file); // rust manages the memory
        data
    }

    #[test]
    fn reading() {
        // TODO move to C++ unittests and split into individual tests
        let (dir, data_dir) = crate::librarydb::tests::data_dir();

        // relative path of library is case insensitive, read works, seek works
        for library in &["data.slf", "DATA.SLF"] {
            let c_data_dir = c_string_from_str(&data_dir.to_str().unwrap());
            let c_data_dir = c_data_dir.as_ptr();
            let c_library = c_string_from_str(&library);
            let c_library = c_library.as_ptr();
            let c_foo_txt = c_string_from_str("foo.txt");
            let c_foo_txt = c_foo_txt.as_ptr();

            let c_ldb = LibraryDB_create(); // must manage the memory
            assert!(LibraryDB_push(c_ldb, c_data_dir, c_library));
            let c_file = LibraryFile_open(c_ldb, c_foo_txt); // must manage the memory
            assert_ne!(c_file, std::ptr::null_mut());
            assert_eq!(LibraryFile_GetPos(c_file), 0);
            let data = read_to_end(c_file);
            assert_eq!(&data, b"data.slf");
            assert!(LibraryFile_Seek(c_file, 0, FILE_SEEK_FROM_START));
            assert_eq!(LibraryFile_GetPos(c_file), 0);
            assert!(LibraryFile_Seek(c_file, 0, FILE_SEEK_FROM_END));
            assert_eq!(LibraryFile_GetPos(c_file), 8);
            assert!(LibraryFile_Seek(c_file, -4, FILE_SEEK_FROM_CURRENT));
            assert_eq!(LibraryFile_GetPos(c_file), 4);
            let data = read_to_end(c_file);
            assert_eq!(&data, b".slf");
            LibraryFile_Close(c_file); // rust manages the memory
            LibraryDB_destroy(c_ldb); // rust manages the memory
        }

        // library order matters, file paths are case insensitive, allow both path separators
        {
            let c_data_dir = c_string_from_str(&data_dir.to_str().unwrap());
            let c_data_dir = c_data_dir.as_ptr();
            let c_foo_slf = c_string_from_str("foo.slf");
            let c_foo_slf = c_foo_slf.as_ptr();
            let c_foobar_slf = c_string_from_str("foobar.slf");
            let c_foobar_slf = c_foobar_slf.as_ptr();

            let c_ldb = LibraryDB_create(); // must manage the memory
            assert!(LibraryDB_push(c_ldb, c_data_dir, c_foo_slf));
            assert!(LibraryDB_push(c_ldb, c_data_dir, c_foobar_slf));
            let data = library_file_data(c_ldb, "foo/bar.txt");
            assert_eq!(&data, b"foo.slf");
            let data = library_file_data(c_ldb, "foo/BAR/baz.txt");
            assert_eq!(&data, b"foo.slf");
            LibraryDB_destroy(c_ldb); // rust manages the memory

            let c_ldb = LibraryDB_create(); // must manage the memory
            assert!(LibraryDB_push(c_ldb, c_data_dir, c_foobar_slf));
            assert!(LibraryDB_push(c_ldb, c_data_dir, c_foo_slf));
            let data = library_file_data(c_ldb, "foo/BAR.TXT");
            assert_eq!(&data, b"foo.slf");
            let data = library_file_data(c_ldb, "foo\\bar/baz.txt");
            assert_eq!(&data, b"foobar.slf");
            LibraryDB_destroy(c_ldb); // rust manages the memory
        }

        dir.close().unwrap();
    }
}
