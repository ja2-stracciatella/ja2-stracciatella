//! This module contains the C interface for [`stracciatella::librarydb`].
//!
//! [`stracciatella::librarydb`]: ../../librarydb/index.html

use std::io::{self, Read, Seek, SeekFrom};

use crate::c::common::*;
use crate::librarydb::{LibraryDB, LibraryFile};

/// Constructor.
/// The caller is responsible for the memory of the library database.
#[no_mangle]
pub extern "C" fn LibraryDB_New() -> *mut LibraryDB {
    let ldb = into_ptr(LibraryDB::new());
    return ldb;
}

/// Destructor.
/// The caller is no longer responsible for the memory of the library database.
#[no_mangle]
pub extern "C" fn LibraryDB_Delete(ldb: *mut LibraryDB) {
    let _drop_me = from_ptr(ldb);
}

/// Adds a library to the end of the library database.
/// Returns true is successful, false otherwise.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn LibraryDB_AddLibrary(
    ldb: *mut LibraryDB,
    data_dir: *const c_char,
    library: *const c_char,
) -> bool {
    let ldb = unsafe_mut(ldb);
    let data_dir = path_from_c_str_or_panic(unsafe_c_str(data_dir));
    let library = path_from_c_str_or_panic(unsafe_c_str(library));
    if let Err(e) = ldb.add_library(data_dir, library) {
        remember_rust_error(format!("{:?}", e));
        return false;
    }
    forget_rust_error();
    return true;
}

/// Opens a library database file for reading.
/// Returns the file on success and null on error.
/// The caller is responsible for the library file memory.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn LibraryFile_Open(
    ldb: *mut LibraryDB,
    path: *const c_char,
) -> *mut LibraryFile {
    let ldb = unsafe_mut(ldb);
    let path = str_from_c_str_or_panic(unsafe_c_str(path));
    let open_result = ldb.open_file(&path);
    if let Err(e) = open_result {
        remember_rust_error(format!("{:?}", e));
        return std::ptr::null_mut();
    }
    forget_rust_error();
    let file = into_ptr(open_result.unwrap());
    return file;
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
        1 => file.seek(SeekFrom::Current(distance)),
        2 => file.seek(SeekFrom::End(distance)),
        _ => Err(io::ErrorKind::InvalidInput.into()),
    };
    if let Err(e) = seek_result {
        remember_rust_error(format!("{:?}", e));
        return false;
    }
    forget_rust_error();
    return true;
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
    if let Err(e) = file.read_exact(buffer) {
        remember_rust_error(format!("{:?}", e));
        return false;
    }
    forget_rust_error();
    return true;
}

/// Gets the current position in a library database file.
#[no_mangle]
pub extern "C" fn LibraryFile_GetPos(file: *mut LibraryFile) -> u64 {
    let file = unsafe_mut(file);
    return file.current_position();
}

/// Gets the size of a library database file.
#[no_mangle]
pub extern "C" fn LibraryFile_GetSize(file: *mut LibraryFile) -> u64 {
    let file = unsafe_mut(file);
    return file.file_size();
}
