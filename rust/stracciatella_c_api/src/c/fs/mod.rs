//! This module contains the C interface for [`stracciatella::fs`].
//!
//! [`stracciatella::fs`]: ../../stracciatella/fs/index.html

use std::ptr;

use stracciatella::fs;

use crate::c::common::*;
use crate::c::vec::VecCString;

pub mod file;
pub mod vfs;

/// Checks if the path exists.
#[unsafe(no_mangle)]
pub extern "C" fn Fs_exists(path: *const c_char) -> bool {
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    fs::metadata(&path).is_ok()
}

/// Checks if the path points to a directory.
#[unsafe(no_mangle)]
pub extern "C" fn Fs_isDir(path: *const c_char) -> bool {
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    fs::metadata(&path).map(|x| x.is_dir()).unwrap_or(false)
}

/// Finds all files in directory
#[unsafe(no_mangle)]
pub extern "C" fn Fs_findAllFilesInDir(
    dir: *const c_char,
    sort_results: bool,
    recursive: bool,
) -> *mut VecCString {
    forget_rust_error();
    let dir = path_buf_from_c_str_or_panic(unsafe_c_str(dir));
    match fs::find_all_files_in_dir(&dir, sort_results, recursive) {
        Err(err) => {
            remember_rust_error(format!("Fs_findAllFilesInDir {:?}: {}", dir, err));
            ptr::null_mut()
        }
        Ok(vec) => {
            let vec: Vec<_> = vec.iter().map(|x| c_string_from_path_or_panic(x)).collect();
            let c_vec = VecCString::from(vec);
            into_ptr(c_vec)
        }
    }
}

/// Finds all directories
#[unsafe(no_mangle)]
pub extern "C" fn Fs_findAllDirsInDir(
    dir: *const c_char,
    sort_results: bool,
    recursive: bool,
) -> *mut VecCString {
    forget_rust_error();
    let dir = path_buf_from_c_str_or_panic(unsafe_c_str(dir));
    match fs::find_all_dirs_in_dir(&dir, sort_results, recursive) {
        Err(err) => {
            remember_rust_error(format!("Fs_findAllDirsInDir {:?}: {}", dir, err));
            ptr::null_mut()
        }
        Ok(vec) => {
            let vec: Vec<_> = vec.iter().map(|x| c_string_from_path_or_panic(x)).collect();
            let c_vec = VecCString::from(vec);
            into_ptr(c_vec)
        }
    }
}

/// Removes a file.
/// Sets the rust error.
/// @see https://doc.rust-lang.org/std/fs/fn.remove_file.html
#[unsafe(no_mangle)]
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
#[unsafe(no_mangle)]
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
#[unsafe(no_mangle)]
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
#[unsafe(no_mangle)]
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

/// Cleans a filename from special characters, so it can be used safely for the filesystem
/// Note that the filename should not contain the extension
#[unsafe(no_mangle)]
pub extern "C" fn Fs_cleanBasename(basename: *const c_char) -> *mut c_char {
    let basename = path_buf_from_c_str_or_panic(unsafe_c_str(basename));
    c_string_from_path_or_panic(&fs::clean_basename(basename)).into_raw()
}
