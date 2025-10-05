use libc::c_char;
use stracciatella::config::EngineOptions;

pub use stracciatella::mods::{Mod, ModManager};

use super::common::{
    c_string_from_str, forget_rust_error, from_ptr, into_ptr, remember_rust_error,
    str_from_c_str_or_panic, unsafe_c_str, unsafe_ref,
};

/// Creates a `ModManager` instance
#[unsafe(no_mangle)]
pub extern "C" fn ModManager_create(engine_options: *const EngineOptions) -> *mut ModManager {
    forget_rust_error();
    let engine_options = unsafe_ref(engine_options);
    let mod_manager = ModManager::new(engine_options);

    match mod_manager {
        Ok(mod_manager) => into_ptr(mod_manager),
        Err(e) => {
            remember_rust_error(format!("{}", e));
            std::ptr::null_mut()
        }
    }
}

/// Creates a `ModManager` instance, without checking wether enabled mods exist
#[unsafe(no_mangle)]
pub extern "C" fn ModManager_createUnchecked(
    engine_options: *const EngineOptions,
) -> *mut ModManager {
    let engine_options = unsafe_ref(engine_options);
    let mod_manager = ModManager::new_unchecked(engine_options);

    into_ptr(mod_manager)
}

/// Destroys the ModManager instance.
/// coverity[+free : arg-0]
#[unsafe(no_mangle)]
pub extern "C" fn ModManager_destroy(mod_manager: *mut ModManager) {
    let _drop_me = from_ptr(mod_manager);
}

/// Gets the number of available mods.
#[unsafe(no_mangle)]
pub extern "C" fn ModManager_getAvailableModsLength(ptr: *const ModManager) -> usize {
    let mod_manager = unsafe_ref(ptr);
    mod_manager.available_mods().len()
}

/// Gets a specific mod by index
#[unsafe(no_mangle)]
pub extern "C" fn ModManager_getAvailableModByIndex(
    ptr: *const ModManager,
    index: usize,
) -> *mut Mod {
    let mod_manager = unsafe_ref(ptr);
    mod_manager
        .available_mods()
        .get(index)
        .map(|v| into_ptr(v.clone()))
        .unwrap_or(std::ptr::null_mut())
}

/// Gets a specific mod by name
#[unsafe(no_mangle)]
pub extern "C" fn ModManager_getAvailableModById(
    ptr: *const ModManager,
    id: *const c_char,
) -> *mut Mod {
    let mod_manager = unsafe_ref(ptr);
    let id = str_from_c_str_or_panic(unsafe_c_str(id));
    mod_manager
        .get_mod_by_id(id)
        .map(|v| into_ptr(v.clone()))
        .unwrap_or(std::ptr::null_mut())
}

/// Destroys the Mod instance.
/// coverity[+free : arg-0]
#[unsafe(no_mangle)]
pub extern "C" fn Mod_destroy(ptr: *mut Mod) {
    let _drop_me = from_ptr(ptr);
}

/// Gets a mods id
#[unsafe(no_mangle)]
pub extern "C" fn Mod_getId(ptr: *const Mod) -> *mut c_char {
    let ptr = unsafe_ref(ptr);
    c_string_from_str(ptr.id()).into_raw()
}

/// Gets a mods name
#[unsafe(no_mangle)]
pub extern "C" fn Mod_getName(ptr: *const Mod) -> *mut c_char {
    let ptr = unsafe_ref(ptr);
    c_string_from_str(ptr.name()).into_raw()
}

/// Gets a mods version string
#[unsafe(no_mangle)]
pub extern "C" fn Mod_getVersionString(ptr: *const Mod) -> *mut c_char {
    let ptr = unsafe_ref(ptr);
    c_string_from_str(ptr.version()).into_raw()
}

/// Gets a mods description
#[unsafe(no_mangle)]
pub extern "C" fn Mod_getDescription(ptr: *const Mod) -> *mut c_char {
    let ptr = unsafe_ref(ptr);
    c_string_from_str(ptr.description()).into_raw()
}
