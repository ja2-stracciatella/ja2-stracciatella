//! This module contains the C interface for [`stracciatella::vfs`].
//!
//! [`stracciatella::vfs`]: ../../../stracciatella/vfs/index.html

use stracciatella::config::EngineOptions;
use stracciatella::mods::ModManager;
use stracciatella::unicode::Nfc;
use stracciatella::vfile::VFile;
use stracciatella::vfs::{Vfs, VfsLayer};

use crate::c::common::*;
use crate::c::vec::VecCString;

/// Creates a virtual filesystem.
/// coverity[+alloc]
#[no_mangle]
pub extern "C" fn Vfs_create() -> *mut Vfs {
    into_ptr(Vfs::new())
}

/// Destroys the virtual filesystem.
/// coverity[+free : arg-0]
#[no_mangle]
pub extern "C" fn Vfs_destroy(vfs: *mut Vfs) {
    let _drop_me = from_ptr(vfs);
}

/// Initializes the VFS based on the information in engine_options
/// Returns true if successful, false otherwise.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn Vfs_init(
    vfs: *mut Vfs,
    engine_options: *const EngineOptions,
    mod_manager: *const ModManager,
) -> bool {
    forget_rust_error();
    let vfs = unsafe_mut(vfs);
    let engine_options = unsafe_ref(engine_options);
    let mod_manager = unsafe_ref(mod_manager);
    if let Err(err) = vfs.init(engine_options, mod_manager) {
        remember_rust_error(format!("Vfs_init_from_engine_options: {}", err));
    }
    no_rust_error()
}

/// Adds an overlay filesystem backed by a filesystem directory.
/// Returns true if successful, false otherwise.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn Vfs_addDir(vfs: *mut Vfs, path: *const c_char) -> bool {
    forget_rust_error();
    let vfs = unsafe_mut(vfs);
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    if let Err(err) = vfs.add_dir(&path) {
        remember_rust_error(format!("Vfs_addDir {:?}: {}", path, err));
    }
    no_rust_error()
}

/// Lists a directory in the VFS with an optional filter on the extension (pass null otherwise).
/// Returns a list of files on success and null otherwise
/// Sets the rust error.
/// coverity[+alloc]
#[no_mangle]
pub extern "C" fn Vfs_readDir(
    vfs: *mut Vfs,
    path: *const c_char,
    extension: *const c_char,
) -> *mut VecCString {
    forget_rust_error();
    let vfs = unsafe_mut(vfs);
    let path = Nfc::caseless_path(str_from_c_str_or_panic(unsafe_c_str(path)));
    let extension = if extension.is_null() {
        None
    } else {
        Some(Nfc::caseless(str_from_c_str_or_panic(unsafe_c_str(
            extension,
        ))))
    };
    let res = match &extension {
        Some(ext) => vfs.read_dir_with_extension(&path, &ext),
        None => vfs.read_dir(&path),
    };
    match res {
        Err(err) => {
            remember_rust_error(format!("VfsFile_readDir {:?}: {}", path, err));
            std::ptr::null_mut()
        }
        Ok(files) => {
            let vec: Vec<_> = files.into_iter().map(|x| c_string_from_str(&x)).collect();
            let c_vec = VecCString::from(vec);
            into_ptr(c_vec)
        }
    }
}

/// Opens a virtual file for reading.
/// Returns the file on success, null otherwise.
/// Sets the rust error.
/// coverity[+alloc]
#[no_mangle]
pub extern "C" fn VfsFile_open(vfs: *mut Vfs, path: *const c_char) -> *mut VFile {
    forget_rust_error();
    let vfs = unsafe_mut(vfs);
    let path = str_from_c_str_or_panic(unsafe_c_str(path));
    match vfs.open(&Nfc::caseless_path(&path)) {
        Err(err) => {
            remember_rust_error(format!("VfsFile_open {:?}: {}", path, err));
            std::ptr::null_mut()
        }
        Ok(file) => into_ptr(file.into()),
    }
}
