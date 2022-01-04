use libc::c_char;

pub use stracciatella::schemas::SchemaManager;

use super::common::{
    c_string_from_str, from_ptr, into_ptr, path_buf_from_c_str_or_panic, unsafe_c_str, unsafe_ref,
};

/// Creates a `SchemaManager` instance
#[no_mangle]
pub extern "C" fn SchemaManager_create() -> *mut SchemaManager {
    into_ptr(SchemaManager::default())
}

/// Destroys the `SchemaManager` instance.
/// coverity[+free : arg-0]
#[no_mangle]
pub extern "C" fn SchemaManager_destroy(mod_manager: *mut SchemaManager) {
    let _drop_me = from_ptr(mod_manager);
}

/// Gets a schema for a path in externalized dir
#[no_mangle]
pub extern "C" fn SchemaManager_getSchemaForPath(
    ptr: *const SchemaManager,
    path: *const c_char,
) -> *mut c_char {
    let schema_manager = unsafe_ref(ptr);
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));

    match schema_manager.get(&path) {
        Some(v) => c_string_from_str(v.as_str()).into_raw(),
        None => std::ptr::null_mut(),
    }
}
