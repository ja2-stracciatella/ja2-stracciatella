//! This module contains the C interface for [`stracciatella::game_settings`].
//!
//! [`stracciatella::game_settings`]: ../../../stracciatella/game_settings/index.html

use libc::c_char;
use stracciatella::config::EngineOptions;
use stracciatella::mods::ModManager;

pub use stracciatella::game_settings::{GameSettingType, GameSettings};

use super::common::{
    c_string_from_path_or_panic, c_string_from_str, forget_rust_error, from_ptr, into_ptr,
    no_rust_error, remember_rust_error, str_from_c_str_or_panic, unsafe_c_str, unsafe_mut,
    unsafe_ref,
};

/// Loads the game settings with the defaults of the mods in the engine options.
/// Returns null and sets the rust error on failure.
/// coverity[+alloc]
#[unsafe(no_mangle)]
pub extern "C" fn GameSettings_load(
    engine_options: *const EngineOptions,
    mod_manager: *const ModManager,
) -> *mut GameSettings {
    forget_rust_error();
    let engine_options = unsafe_ref(engine_options);
    let mod_manager = unsafe_ref(mod_manager);
    match GameSettings::load(engine_options, mod_manager) {
        Ok(settings) => into_ptr(settings),
        Err(e) => {
            remember_rust_error(format!("GameSettings_load: {}", e));
            std::ptr::null_mut()
        }
    }
}

/// Destroys the game settings.
/// coverity[+free : arg-0]
#[unsafe(no_mangle)]
pub extern "C" fn GameSettings_destroy(ptr: *mut GameSettings) {
    let _drop_me = from_ptr(ptr);
}

/// Gets the number of sections.
#[unsafe(no_mangle)]
pub extern "C" fn GameSettings_getSectionsLength(ptr: *const GameSettings) -> usize {
    unsafe_ref(ptr).sections().len()
}

/// Gets the title of a section, or null for settings outside of a section.
/// The caller is responsible for the returned memory.
#[unsafe(no_mangle)]
pub extern "C" fn GameSettings_getSectionTitle(
    ptr: *const GameSettings,
    section: usize,
) -> *mut c_char {
    match &unsafe_ref(ptr).sections()[section] {
        Some(title) => c_string_from_str(title).into_raw(),
        None => std::ptr::null_mut(),
    }
}

/// Gets the number of settings.
#[unsafe(no_mangle)]
pub extern "C" fn GameSettings_getSettingsLength(ptr: *const GameSettings) -> usize {
    unsafe_ref(ptr).settings().len()
}

/// Gets the index of the section of a setting.
#[unsafe(no_mangle)]
pub extern "C" fn GameSettings_getSettingSection(ptr: *const GameSettings, index: usize) -> usize {
    unsafe_ref(ptr).settings()[index].section()
}

/// Gets the title of a setting.
/// The caller is responsible for the returned memory.
#[unsafe(no_mangle)]
pub extern "C" fn GameSettings_getSettingTitle(
    ptr: *const GameSettings,
    index: usize,
) -> *mut c_char {
    c_string_from_str(unsafe_ref(ptr).settings()[index].title()).into_raw()
}

/// Gets the description of a setting, or null if it has none.
/// The caller is responsible for the returned memory.
#[unsafe(no_mangle)]
pub extern "C" fn GameSettings_getSettingDescription(
    ptr: *const GameSettings,
    index: usize,
) -> *mut c_char {
    match unsafe_ref(ptr).settings()[index].description() {
        Some(d) => c_string_from_str(d).into_raw(),
        None => std::ptr::null_mut(),
    }
}

/// Gets a text describing the allowed values of a setting, or null if any value of its type is allowed.
/// The caller is responsible for the returned memory.
#[unsafe(no_mangle)]
pub extern "C" fn GameSettings_getSettingAllowedValues(
    ptr: *const GameSettings,
    index: usize,
) -> *mut c_char {
    match unsafe_ref(ptr).settings()[index].allowed_values() {
        Some(a) => c_string_from_str(a).into_raw(),
        None => std::ptr::null_mut(),
    }
}

/// Gets the type of a setting.
#[unsafe(no_mangle)]
pub extern "C" fn GameSettings_getSettingType(
    ptr: *const GameSettings,
    index: usize,
) -> GameSettingType {
    unsafe_ref(ptr).settings()[index].setting_type()
}

/// Gets the text representation of the current value of a setting.
/// Booleans are "true" or "false".
/// The caller is responsible for the returned memory.
#[unsafe(no_mangle)]
pub extern "C" fn GameSettings_getSettingValue(
    ptr: *const GameSettings,
    index: usize,
) -> *mut c_char {
    c_string_from_str(&unsafe_ref(ptr).value_text_of(index)).into_raw()
}

/// Whether the player changed the value of a setting.
#[unsafe(no_mangle)]
pub extern "C" fn GameSettings_isSettingModified(ptr: *const GameSettings, index: usize) -> bool {
    unsafe_ref(ptr).is_modified(index)
}

/// Sets the value of a setting from its text representation.
/// Returns false if it is not a valid value for the setting.
#[unsafe(no_mangle)]
pub extern "C" fn GameSettings_setSettingValue(
    ptr: *mut GameSettings,
    index: usize,
    value: *const c_char,
) -> bool {
    let value = str_from_c_str_or_panic(unsafe_c_str(value));
    unsafe_mut(ptr).set(index, value)
}

/// Resets all settings to their defaults.
#[unsafe(no_mangle)]
pub extern "C" fn GameSettings_resetAll(ptr: *mut GameSettings) {
    unsafe_mut(ptr).reset_all();
}

/// Writes the changed values to the patch file.
/// Returns false and sets the rust error on failure.
#[unsafe(no_mangle)]
pub extern "C" fn GameSettings_save(ptr: *mut GameSettings) -> bool {
    forget_rust_error();
    if let Err(e) = unsafe_mut(ptr).save() {
        remember_rust_error(format!("GameSettings_save: {}", e));
    }
    no_rust_error()
}

/// Gets the path of the patch file the changed values are written to.
/// The caller is responsible for the returned memory.
#[unsafe(no_mangle)]
pub extern "C" fn GameSettings_getPatchPath(ptr: *const GameSettings) -> *mut c_char {
    c_string_from_path_or_panic(unsafe_ref(ptr).patch_path()).into_raw()
}
