//! This module contains the C interface for [`stracciatella::fs::TempDir`].

use std::ptr;

use stracciatella::fs;

use crate::c::common::*;

/// A directory in the filesystem that is automatically deleted.
/// The contents of the directory are deleted before the directory is deleted.
pub struct TempDir {
    inner: fs::TempDir,
}

/// Attempts to create a temporary directory inside the temporary directory of the operating system.
/// Returns null on error.
/// Sets the rust error.
/// @see https://doc.rust-lang.org/std/env/fn.temp_dir.html
#[unsafe(no_mangle)]
pub extern "C" fn TempDir_create() -> *mut TempDir {
    forget_rust_error();
    let mut builder = fs::TempBuilder::new();
    builder.prefix("ja2-stracciatella-");
    builder.rand_bytes(8);

    #[cfg(target_os = "android")]
    let tempdir = {
        use stracciatella::android;
        android::get_android_cache_dir()
            .map_err(|e| e.to_string())
            .and_then(|v| builder.tempdir_in(v).map_err(|e| e.to_string()))
    };
    #[cfg(not(target_os = "android"))]
    let tempdir = builder.tempdir().map_err(|e| e.to_string());

    match tempdir {
        Err(err) => {
            remember_rust_error(format!("TempDir_create: {}", err));
            ptr::null_mut()
        }
        Ok(tempdir) => into_ptr(TempDir { inner: tempdir }),
    }
}

/// Destroys the temporary directory.
/// This is a best effort approach, in case of error the directory might not be deleted.
/// Sets the rust error.
#[unsafe(no_mangle)]
pub extern "C" fn TempDir_destroy(tempdir: *mut TempDir) -> bool {
    forget_rust_error();
    let tempdir = from_ptr(tempdir);
    if let Err(err) = tempdir.inner.close() {
        remember_rust_error(format!("TempDir_destroy: {}", err));
    }
    no_rust_error()
}

/// Gets the path of the temporary directory.
#[unsafe(no_mangle)]
pub extern "C" fn TempDir_path(tempdir: *mut TempDir) -> *mut c_char {
    let tempdir = unsafe_ref(tempdir);
    let path = tempdir.inner.path();
    let c_path = c_string_from_path_or_panic(path);
    c_path.into_raw()
}
