//! This module contains the C interface for a single [`stracciatella::vfs::VfsLayer`].
//!
//! The C++ `DirFs` class is a thin wrapper around one of these handles, so that the engine gets
//! the case insensitive path handling of the VFS for the directories it writes to as well.

use std::path::PathBuf;
use std::ptr;
use std::sync::Arc;

use stracciatella::unicode::Nfc;
use stracciatella::vfile::VFile;
use stracciatella::vfs::dir::DirFs;
use stracciatella::vfs::tmp::TmpFs;
use stracciatella::vfs::{VfsLayer as VfsLayerTrait, VfsOpenOptions};

use crate::c::common::*;
use crate::c::fs::file::{
    FILE_OPEN_APPEND, FILE_OPEN_CREATE, FILE_OPEN_CREATE_NEW, FILE_OPEN_READ, FILE_OPEN_TRUNCATE,
    FILE_OPEN_WRITE,
};
use crate::c::vec::VecCString;

/// A handle to a single VFS layer.
pub struct VfsLayer {
    layer: Arc<dyn VfsLayerTrait>,
    /// The filesystem directory backing this layer, if it has one.
    ///
    /// Lets the engine move a file between two directory backed layers with a plain rename instead
    /// of copying its contents.
    dir_path: Option<PathBuf>,
}

impl VfsLayer {
    fn new(layer: Arc<dyn VfsLayerTrait>, dir_path: Option<PathBuf>) -> VfsLayer {
        VfsLayer { layer, dir_path }
    }
}

/// Opens a layer backed by a filesystem directory.
/// Returns the layer on success, null otherwise.
/// Sets the rust error.
/// coverity[+alloc]
#[unsafe(no_mangle)]
pub extern "C" fn VfsLayer_fromDir(path: *const c_char, writable: bool) -> *mut VfsLayer {
    forget_rust_error();
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    match DirFs::with_mode(&path, writable) {
        Err(err) => {
            remember_rust_error(format!("VfsLayer_fromDir {:?}: {}", path, err));
            ptr::null_mut()
        }
        Ok(layer) => into_ptr(VfsLayer::new(layer, Some(path))),
    }
}

/// Opens a writable layer backed by a fresh temporary directory.
/// The directory and its contents are deleted when the layer is closed.
/// Returns the layer on success, null otherwise.
/// Sets the rust error.
/// coverity[+alloc]
#[unsafe(no_mangle)]
pub extern "C" fn VfsLayer_fromTempDir() -> *mut VfsLayer {
    forget_rust_error();
    match TmpFs::new() {
        Err(err) => {
            remember_rust_error(format!("VfsLayer_fromTempDir: {}", err));
            ptr::null_mut()
        }
        Ok(layer) => {
            let dir_path = layer.path().to_owned();
            into_ptr(VfsLayer::new(layer, Some(dir_path)))
        }
    }
}

/// Closes the layer.
/// coverity[+free : arg-0]
#[unsafe(no_mangle)]
pub extern "C" fn VfsLayer_close(layer: *mut VfsLayer) {
    let _drop_me = from_ptr(layer);
}

/// Returns the filesystem directory backing the layer, or null when it has none.
/// The caller is responsible for the memory.
/// coverity[+alloc]
#[unsafe(no_mangle)]
pub extern "C" fn VfsLayer_dirPath(layer: *mut VfsLayer) -> *mut c_char {
    let layer = unsafe_ref(layer);
    match &layer.dir_path {
        Some(dir_path) => c_string_from_path_or_panic(dir_path).into_raw(),
        None => ptr::null_mut(),
    }
}

/// Opens a file in the layer according to the options.
/// Returns the file on success, null otherwise.
/// Sets the rust error.
/// @see FILE_OPEN_*
/// coverity[+alloc]
#[unsafe(no_mangle)]
pub extern "C" fn VfsLayer_open(
    layer: *mut VfsLayer,
    path: *const c_char,
    options: u8,
) -> *mut VFile {
    forget_rust_error();
    let layer = unsafe_ref(layer);
    let path = vfs_path(path);
    let raw_options = options;
    let options = VfsOpenOptions {
        read: (options & FILE_OPEN_READ) != 0,
        write: (options & FILE_OPEN_WRITE) != 0,
        append: (options & FILE_OPEN_APPEND) != 0,
        truncate: (options & FILE_OPEN_TRUNCATE) != 0,
        create: (options & FILE_OPEN_CREATE) != 0,
        create_new: (options & FILE_OPEN_CREATE_NEW) != 0,
    };
    match layer.layer.open_with_options(&path, options) {
        Err(err) => {
            remember_rust_error(format!(
                "VfsLayer_open {:?} {:#02x}: {}",
                path, raw_options, err
            ));
            ptr::null_mut()
        }
        // A file opened for reading is buffered, exactly like `File_open` does it.
        Ok(file) => into_ptr(if options.is_write() {
            VFile::writable_vfs_file(file)
        } else {
            VFile::from(file)
        }),
    }
}

/// Removes a file from the layer.
/// Returns true if successful, false otherwise.
/// Sets the rust error.
#[unsafe(no_mangle)]
pub extern "C" fn VfsLayer_removeFile(layer: *mut VfsLayer, path: *const c_char) -> bool {
    forget_rust_error();
    let layer = unsafe_ref(layer);
    let path = vfs_path(path);
    if let Err(err) = layer.layer.remove_file(&path) {
        remember_rust_error(format!("VfsLayer_removeFile {:?}: {}", path, err));
    }
    no_rust_error()
}

/// Creates a directory in the layer, including missing parents.
/// Returns true if successful, false otherwise.
/// Sets the rust error.
#[unsafe(no_mangle)]
pub extern "C" fn VfsLayer_createDir(layer: *mut VfsLayer, path: *const c_char) -> bool {
    forget_rust_error();
    let layer = unsafe_ref(layer);
    let path = vfs_path(path);
    if let Err(err) = layer.layer.create_dir(&path) {
        remember_rust_error(format!("VfsLayer_createDir {:?}: {}", path, err));
    }
    no_rust_error()
}

/// Removes all files in a directory, leaving subdirectories alone.
/// Returns true if successful, false otherwise.
/// Sets the rust error.
#[unsafe(no_mangle)]
pub extern "C" fn VfsLayer_eraseDir(layer: *mut VfsLayer, path: *const c_char) -> bool {
    forget_rust_error();
    let layer = unsafe_ref(layer);
    let path = vfs_path(path);
    if let Err(err) = layer.layer.erase_dir(&path) {
        remember_rust_error(format!("VfsLayer_eraseDir {:?}: {}", path, err));
    }
    no_rust_error()
}

/// Renames or moves a path within the layer.
/// Returns true if successful, false otherwise.
/// Sets the rust error.
#[unsafe(no_mangle)]
pub extern "C" fn VfsLayer_rename(
    layer: *mut VfsLayer,
    from: *const c_char,
    to: *const c_char,
) -> bool {
    forget_rust_error();
    let layer = unsafe_ref(layer);
    let from = vfs_path(from);
    let to = vfs_path(to);
    if let Err(err) = layer.layer.rename(&from, &to) {
        remember_rust_error(format!("VfsLayer_rename {:?} {:?}: {}", from, to, err));
    }
    no_rust_error()
}

/// Lists the files in a directory of the layer, with an optional filter on the extension (pass
/// null otherwise). The returned paths are relative to the directory.
/// Returns the list on success, null otherwise.
/// Sets the rust error.
/// coverity[+alloc]
#[unsafe(no_mangle)]
pub extern "C" fn VfsLayer_findAllFilesInDir(
    layer: *mut VfsLayer,
    path: *const c_char,
    extension: *const c_char,
    recursive: bool,
) -> *mut VecCString {
    forget_rust_error();
    let layer = unsafe_ref(layer);
    let path = vfs_path(path);
    let extension = if extension.is_null() {
        None
    } else {
        Some(Nfc::caseless(&format!(
            ".{}",
            str_from_c_str_or_panic(unsafe_c_str(extension))
        )))
    };
    match layer.layer.find_all_files_in_dir(&path, recursive) {
        Err(err) => {
            remember_rust_error(format!("VfsLayer_findAllFilesInDir {:?}: {}", path, err));
            ptr::null_mut()
        }
        Ok(files) => {
            let vec: Vec<_> = files
                .into_iter()
                .filter(|file| match &extension {
                    Some(extension) => file.ends_with(extension.as_str()),
                    None => true,
                })
                .map(|file| c_string_from_str(&file))
                .collect();
            into_ptr(VecCString::from(vec))
        }
    }
}

/// Lists the directories in a directory of the layer.
/// The returned paths are relative to the directory.
/// Returns the list on success, null otherwise.
/// Sets the rust error.
/// coverity[+alloc]
#[unsafe(no_mangle)]
pub extern "C" fn VfsLayer_findAllDirsInDir(
    layer: *mut VfsLayer,
    path: *const c_char,
    recursive: bool,
) -> *mut VecCString {
    forget_rust_error();
    let layer = unsafe_ref(layer);
    let path = vfs_path(path);
    match layer.layer.find_all_dirs_in_dir(&path, recursive) {
        Err(err) => {
            remember_rust_error(format!("VfsLayer_findAllDirsInDir {:?}: {}", path, err));
            ptr::null_mut()
        }
        Ok(dirs) => {
            let vec: Vec<_> = dirs
                .into_iter()
                .map(|dir| c_string_from_str(&dir))
                .collect();
            into_ptr(VecCString::from(vec))
        }
    }
}

/// Resolves the existing components of a path case insensitively.
/// The caller is responsible for the memory.
/// coverity[+alloc]
#[unsafe(no_mangle)]
pub extern "C" fn VfsLayer_resolveExistingComponents(
    layer: *mut VfsLayer,
    path: *const c_char,
) -> *mut c_char {
    let layer = unsafe_ref(layer);
    let path = vfs_path(path);
    let resolved = layer.layer.resolve_existing_components(&path);
    c_string_from_str(&resolved).into_raw()
}

/// Returns true if the path exists in the layer.
#[unsafe(no_mangle)]
pub extern "C" fn VfsLayer_exists(layer: *mut VfsLayer, path: *const c_char) -> bool {
    let layer = unsafe_ref(layer);
    layer.layer.exists(&vfs_path(path)).unwrap_or(false)
}

/// Returns true if the path exists in the layer and is a file.
#[unsafe(no_mangle)]
pub extern "C" fn VfsLayer_isFile(layer: *mut VfsLayer, path: *const c_char) -> bool {
    let layer = unsafe_ref(layer);
    layer
        .layer
        .metadata(&vfs_path(path))
        .map(|metadata| !metadata.is_dir)
        .unwrap_or(false)
}

/// Returns true if the path exists in the layer and is a directory.
#[unsafe(no_mangle)]
pub extern "C" fn VfsLayer_isDir(layer: *mut VfsLayer, path: *const c_char) -> bool {
    let layer = unsafe_ref(layer);
    layer
        .layer
        .metadata(&vfs_path(path))
        .map(|metadata| metadata.is_dir)
        .unwrap_or(false)
}

/// Gets whether a path is read only.
/// Returns true if successful, false otherwise.
/// Sets the rust error.
#[unsafe(no_mangle)]
pub extern "C" fn VfsLayer_getReadOnly(
    layer: *mut VfsLayer,
    path: *const c_char,
    readonly: *mut bool,
) -> bool {
    forget_rust_error();
    let layer = unsafe_ref(layer);
    let path = vfs_path(path);
    match layer.layer.metadata(&path) {
        Err(err) => remember_rust_error(format!("VfsLayer_getReadOnly {:?}: {}", path, err)),
        Ok(metadata) => *unsafe_mut(readonly) = metadata.read_only,
    }
    no_rust_error()
}

/// Gets the last modification time of a path in seconds since the unix epoch.
/// Returns true if successful, false otherwise.
/// Sets the rust error.
#[unsafe(no_mangle)]
pub extern "C" fn VfsLayer_modifiedSecs(
    layer: *mut VfsLayer,
    path: *const c_char,
    modified_secs: *mut f64,
) -> bool {
    forget_rust_error();
    let layer = unsafe_ref(layer);
    let path = vfs_path(path);
    match layer.layer.metadata(&path) {
        Err(err) => remember_rust_error(format!("VfsLayer_modifiedSecs {:?}: {}", path, err)),
        Ok(metadata) => match metadata.modified_secs {
            Some(secs) => *unsafe_mut(modified_secs) = secs,
            None => remember_rust_error(format!(
                "VfsLayer_modifiedSecs {:?}: no modification time",
                path
            )),
        },
    }
    no_rust_error()
}

/// Gets the amount of free space in bytes available to the layer.
/// Returns true if successful, false otherwise.
/// Sets the rust error.
#[unsafe(no_mangle)]
pub extern "C" fn VfsLayer_freeSpace(layer: *mut VfsLayer, bytes: *mut u64) -> bool {
    forget_rust_error();
    let layer = unsafe_ref(layer);
    match layer.layer.free_space() {
        Err(err) => remember_rust_error(format!("VfsLayer_freeSpace: {}", err)),
        Ok(free_space) => *unsafe_mut(bytes) = free_space,
    }
    no_rust_error()
}

/// Converts a path from C into the form the VFS uses.
///
/// The engine passes native paths, which use a backslash separator on windows. The case is kept,
/// because a file created through this API has to end up on disk with the name the engine asked
/// for. Looking a path up stays case insensitive, the layers fold the case themselves.
fn vfs_path(path: *const c_char) -> Nfc {
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    let path = path
        .components()
        .filter_map(|component| match component {
            std::path::Component::Normal(part) => part.to_str(),
            _ => None,
        })
        .collect::<Vec<_>>()
        .join("/");
    Nfc::path(&path)
}
