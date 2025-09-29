use libc::c_char;

pub use stracciatella::schemas::SchemaManager;

use super::{
    common::{
        c_string_from_str, from_ptr, into_ptr, path_buf_from_c_str_or_panic, unsafe_c_str,
        unsafe_ref,
    },
    json::RJsonValue,
    vec::VecCString,
};

/// Creates a `SchemaManager` instance
#[unsafe(no_mangle)]
pub extern "C" fn SchemaManager_create() -> *mut SchemaManager {
    into_ptr(SchemaManager::default())
}

/// Destroys the `SchemaManager` instance.
/// coverity[+free : arg-0]
#[unsafe(no_mangle)]
pub extern "C" fn SchemaManager_destroy(mod_manager: *mut SchemaManager) {
    let _drop_me = from_ptr(mod_manager);
}

/// Gets a schema for a path in externalized dir
#[unsafe(no_mangle)]
pub extern "C" fn SchemaManager_validateValueForPath(
    ptr: *const SchemaManager,
    path: *const c_char,
    value: *const RJsonValue,
) -> *mut VecCString {
    let schema_manager = unsafe_ref(ptr);
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    let value = unsafe_ref(value);

    match schema_manager.validate(&path, &value.0) {
        Some(v) => into_ptr(VecCString::from(
            v.into_iter()
                .map(|s| c_string_from_str(&s))
                .collect::<Vec<_>>(),
        )),
        None => std::ptr::null_mut(),
    }
}
