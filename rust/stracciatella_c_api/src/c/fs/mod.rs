//! This module contains the C interface for [`stracciatella::fs`].
//!
//! [`stracciatella::fs`]: ../../stracciatella/fs/index.html

use std::f64;
use std::ptr;
use std::time;
use std::u64;
use std::usize;

use stracciatella::fs;

use crate::c::common::*;
use crate::c::vec::{VecCString, VecU8};

pub mod file;
pub mod tempdir;

/// Creates a directory.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn Fs_createDir(path: *const c_char) -> bool {
    forget_rust_error();
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    if let Err(err) = fs::create_dir(&path) {
        remember_rust_error(format!("Fs_createDir {:?}: {}", path, err));
    }
    no_rust_error()
}

/// Checks if the path exists.
#[no_mangle]
pub extern "C" fn Fs_exists(path: *const c_char) -> bool {
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    fs::metadata(&path).is_ok()
}

/// Gets the free space in the target path.
/// On error the free space will be 0.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn Fs_freeSpace(path: *const c_char, bytes: *mut u64) -> bool {
    forget_rust_error();
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    let bytes = unsafe_mut(bytes);
    let free_space = match fs::free_space(&path) {
        Err(err) => {
            remember_rust_error(format!("Fs_freeSpace {:?}: {}", path, err));
            0
        }
        Ok(n) => n,
    };
    *bytes = free_space;
    no_rust_error()
}

/// Checks if the path points to a directory.
#[no_mangle]
pub extern "C" fn Fs_isDir(path: *const c_char) -> bool {
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    fs::metadata(&path).map(|x| x.is_dir()).unwrap_or(false)
}

/// Checks if the path points to a file.
#[no_mangle]
pub extern "C" fn Fs_isFile(path: *const c_char) -> bool {
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    fs::metadata(&path).map(|x| x.is_file()).unwrap_or(false)
}

/// Gets the paths of the directory entries.
/// Returns null on error.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn Fs_readDirPaths(
    dir: *const c_char,
    ignore_entry_errors: bool,
) -> *mut VecCString {
    forget_rust_error();
    let dir = path_buf_from_c_str_or_panic(unsafe_c_str(dir));
    match fs::read_dir_paths(&dir, ignore_entry_errors) {
        Err(err) => {
            remember_rust_error(format!("Fs_readDirPaths {:?}: {}", dir, err));
            ptr::null_mut()
        }
        Ok(vec) => {
            let vec: Vec<_> = vec
                .iter()
                .map(|x| c_string_from_path_or_panic(&x))
                .collect();
            let c_vec = VecCString::from(vec);
            into_ptr(c_vec)
        }
    }
}

/// Gets the modified time in seconds since the unix epoch.
/// On error the modified time will be the minimum value.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn Fs_modifiedSecs(path: *const c_char, modified_secs: *mut f64) -> bool {
    forget_rust_error();
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    let modified_secs = unsafe_mut(modified_secs);
    // FIXME use Duration.as_secs_f64 with rust 1.38.0+
    let as_secs_f64 = |duration: time::Duration| {
        const NANOSECOND: f64 = 0.000_000_001;
        duration.as_secs() as f64 + f64::from(duration.subsec_nanos()) * NANOSECOND
    };
    let secs_result = fs::metadata(&path).and_then(|x| x.modified()).map(|x| {
        match x.duration_since(time::UNIX_EPOCH) {
            Ok(duration) => as_secs_f64(duration),
            Err(err) => -as_secs_f64(err.duration()),
        }
    });
    match secs_result {
        Err(err) => {
            remember_rust_error(format!("Fs_modifiedSecs {:?}: {}", path, err));
            *modified_secs = f64::MIN;
        }
        Ok(secs) => {
            *modified_secs = secs;
        }
    }
    no_rust_error()
}

/// Removes a directory and all it's contents.
/// Sets the rust error.
/// @see https://doc.rust-lang.org/std/fs/fn.remove_dir_all.html
#[no_mangle]
pub extern "C" fn Fs_removeDirAll(dir: *const c_char) -> bool {
    forget_rust_error();
    let dir = path_buf_from_c_str_or_panic(unsafe_c_str(dir));
    if let Err(err) = fs::remove_dir_all(&dir) {
        remember_rust_error(format!("Fs_removeDirAll {:?}: {}", dir, err));
    }
    no_rust_error()
}

/// Removes a file.
/// Sets the rust error.
/// @see https://doc.rust-lang.org/std/fs/fn.remove_file.html
#[no_mangle]
pub extern "C" fn Fs_removeFile(path: *const c_char) -> bool {
    forget_rust_error();
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    if let Err(err) = fs::remove_file(&path) {
        remember_rust_error(format!("Fs_removeFile {:?}: {}", path, err));
    }
    no_rust_error()
}

/// Renames a file or directory.
/// Sets the rust error.
/// @see https://doc.rust-lang.org/std/fs/fn.rename.html
#[no_mangle]
pub extern "C" fn Fs_rename(from: *const c_char, to: *const c_char) -> bool {
    forget_rust_error();
    let from = path_buf_from_c_str_or_panic(unsafe_c_str(from));
    let to = path_buf_from_c_str_or_panic(unsafe_c_str(to));
    if let Err(err) = fs::rename(&from, &to) {
        remember_rust_error(format!("Fs_rename {:?} {:?}: {}", from, to, err));
    }
    no_rust_error()
}

/// Returns base (optional) joined with path.
/// The path separators are normalized and path components are resolved only when needed.
/// The returned path might or might not exist.
#[no_mangle]
pub extern "C" fn Fs_resolveExistingComponents(
    path: *const c_char,
    base: *const c_char,
    caseless: bool,
) -> *mut c_char {
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    let resolved = if base.is_null() {
        fs::resolve_existing_components(&path, None, caseless)
    } else {
        let base = path_buf_from_c_str_or_panic(unsafe_c_str(base));
        fs::resolve_existing_components(&path, Some(&base), caseless)
    };
    c_string_from_path_or_panic(&resolved).into_raw()
}

/// Get the readonly permissions of a file or directory.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn Fs_getReadOnly(path: *const c_char, readonly: *mut bool) -> bool {
    forget_rust_error();
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    let readonly = unsafe_mut(readonly);
    let result = fs::metadata(&path).map(|x| {
        *readonly = x.permissions().readonly();
    });
    if let Err(err) = result {
        remember_rust_error(format!("Fs_getReadOnly {:?} {}: {}", path, readonly, err));
    }
    no_rust_error()
}

/// Sets the readonly permissions of a file or directory.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn Fs_setReadOnly(path: *const c_char, readonly: bool) -> bool {
    forget_rust_error();
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    let result = fs::metadata(&path).and_then(|x| {
        let mut permissions = x.permissions();
        permissions.set_readonly(readonly);
        fs::set_permissions(&path, permissions)
    });
    if let Err(err) = result {
        remember_rust_error(format!("Fs_setReadOnly {:?} {}: {}", path, readonly, err));
    }
    no_rust_error()
}

/// Reads all the bytes from a file.
/// Returns null if there is an error.
/// Sets the rust error.
/// @see https://doc.rust-lang.org/std/fs/fn.read.html
#[no_mangle]
pub extern "C" fn Fs_read(path: *const c_char) -> *mut VecU8 {
    forget_rust_error();
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    match fs::read(&path) {
        Err(err) => {
            remember_rust_error(format!("Fs_read {:?}: {}", path, err));
            ptr::null_mut()
        }
        Ok(vec) => into_ptr(VecU8::from(vec)),
    }
}

/// Writes the bytes to a file.
/// The file will be created if it does not exist.
/// Sets the rust error.
/// @see https://doc.rust-lang.org/std/fs/fn.write.html
#[no_mangle]
pub extern "C" fn Fs_write(path: *const c_char, buf: *const u8, buf_len: usize) -> bool {
    forget_rust_error();
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    let buf = unsafe_slice(buf, buf_len);
    if let Err(err) = fs::write(&path, buf) {
        remember_rust_error(format!("Fs_write {:?} {}: {}", path, buf_len, err));
    }
    no_rust_error()
}
