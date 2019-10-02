//! This module contains the C interface for [`stracciatella::librarydb`].
//!
//! [`stracciatella::librarydb`]: ../../../stracciatella/librarydb/index.html

use std::io;
use std::io::{Read, Seek, SeekFrom};

use stracciatella::librarydb::{LibraryDB, LibraryFile};

use crate::c::common::*;

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
pub extern "C" fn LibraryFile_close(file: *mut LibraryFile) {
    let _drop_me = from_ptr(file);
}

/// Seeks a library database file.
/// `from` expects FileSeekMode values (see src/sgp/SGPFile.h)
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn LibraryFile_seek(file: *mut LibraryFile, distance: i64, from: c_int) -> bool {
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
pub extern "C" fn LibraryFile_read(
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
pub extern "C" fn LibraryFile_getPosition(file: *mut LibraryFile) -> u64 {
    let file = unsafe_mut(file);
    file.current_position()
}

/// Gets the size of a library database file.
#[no_mangle]
pub extern "C" fn LibraryFile_getSize(file: *mut LibraryFile) -> u64 {
    let file = unsafe_mut(file);
    file.file_size()
}
