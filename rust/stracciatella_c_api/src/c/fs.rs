//! This module contains the C interface for [`stracciatella::fs`].
//!
//! [`stracciatella::fs`]: ../../stracciatella/fs/index.html

use std::f64;
use std::ptr;
use std::time;

use stracciatella::fs;

use crate::c::common::*;
use crate::c::misc::VecCString;

/// Checks if the path exists.
#[no_mangle]
pub extern "C" fn Fs_exists(path: *const c_char) -> bool {
    let path = path_from_c_str_or_panic(unsafe_c_str(path));
    fs::metadata(&path).is_ok()
}

/// Checks if the path points to a file.
#[no_mangle]
pub extern "C" fn Fs_isFile(path: *const c_char) -> bool {
    let path = path_from_c_str_or_panic(unsafe_c_str(path));
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
    let dir = path_from_c_str_or_panic(unsafe_c_str(dir));
    match fs::read_dir_paths(&dir, ignore_entry_errors) {
        Err(err) => {
            remember_rust_error(format!("Fs_readDirPaths {:?}: {}", dir, err));
            ptr::null_mut()
        }
        Ok(vec) => {
            let vec = vec
                .iter()
                .map(|x| c_string_from_path_or_panic(&x))
                .collect();
            let c_vec = VecCString::from_vec(vec);
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
    let path = path_from_c_str_or_panic(unsafe_c_str(path));
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
    let dir = path_from_c_str_or_panic(unsafe_c_str(dir));
    if let Err(err) = fs::remove_dir_all(&dir) {
        remember_rust_error(format!("Fs_removeDirAll {:?}: {}", dir, err));
    }
    no_rust_error()
}

/// Renames a file or directory.
/// Sets the rust error.
/// @see https://doc.rust-lang.org/std/fs/fn.rename.html
#[no_mangle]
pub extern "C" fn Fs_rename(from: *const c_char, to: *const c_char) -> bool {
    forget_rust_error();
    let from = path_from_c_str_or_panic(unsafe_c_str(from));
    let to = path_from_c_str_or_panic(unsafe_c_str(to));
    if let Err(err) = fs::rename(&from, &to) {
        remember_rust_error(format!("Fs_rename {:?} {:?}: {}", from, to, err));
    }
    no_rust_error()
}
