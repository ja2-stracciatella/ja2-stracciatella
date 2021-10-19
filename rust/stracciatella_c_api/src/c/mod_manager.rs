use libc::c_char;
use stracciatella::{config::EngineOptions, get_assets_dir};

pub use stracciatella::mods::{Mod, ModManager};

use super::common::{
    c_string_from_str, from_ptr, into_ptr, str_from_c_str_or_panic, unsafe_c_str, unsafe_ref,
};

/// Creates a `ModManager` instance
#[no_mangle]
pub extern "C" fn ModManager_create(engine_options: *const EngineOptions) -> *mut ModManager {
    let assets_dir = get_assets_dir();
    let engine_options = unsafe_ref(engine_options);

    into_ptr(ModManager::new(
        &engine_options.stracciatella_home,
        &assets_dir,
    ))
}

/// Destroys the ModManager instance.
/// coverity[+free : arg-0]
#[no_mangle]
pub extern "C" fn ModManager_destroy(mod_manager: *mut ModManager) {
    let _drop_me = from_ptr(mod_manager);
}

/// Gets the number of available mods.
#[no_mangle]
pub extern "C" fn ModManager_getAvailableModsLength(ptr: *const ModManager) -> usize {
    let mod_manager = unsafe_ref(ptr);
    mod_manager.available_mods().len()
}

/// Gets a specific mod by index
#[no_mangle]
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
#[no_mangle]
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
#[no_mangle]
pub extern "C" fn Mod_destroy(ptr: *mut Mod) {
    let _drop_me = from_ptr(ptr);
}

/// Gets a mods id
#[no_mangle]
pub extern "C" fn Mod_getId(ptr: *const Mod) -> *mut c_char {
    let ptr = unsafe_ref(ptr);
    c_string_from_str(ptr.id()).into_raw()
}

/// Gets a mods name
#[no_mangle]
pub extern "C" fn Mod_getName(ptr: *const Mod) -> *mut c_char {
    let ptr = unsafe_ref(ptr);
    c_string_from_str(ptr.name()).into_raw()
}

/// Gets a mods version string
#[no_mangle]
pub extern "C" fn Mod_getVersionString(ptr: *const Mod) -> *mut c_char {
    let ptr = unsafe_ref(ptr);
    c_string_from_str(ptr.version()).into_raw()
}

/// Gets a mods version string
#[no_mangle]
pub extern "C" fn Mod_getDescription(ptr: *const Mod) -> *mut c_char {
    let ptr = unsafe_ref(ptr);
    c_string_from_str(ptr.description()).into_raw()
}
