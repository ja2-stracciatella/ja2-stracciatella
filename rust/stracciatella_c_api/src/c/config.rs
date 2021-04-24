//! This module contains the C interface for [`stracciatella::config`].
//!
//! [`stracciatella::config`]: ../../stracciatella/config/index.html

use std::ptr;

use stracciatella::config::{
    find_stracciatella_home, Cli, EngineOptions, Ja2Json, Resolution, ScalingQuality,
    VanillaVersion,
};

use crate::c::common::*;

/// Creates `EngineOptions` with the provided command line arguments.
/// Loads values from `(stracciatella_home)/ja2.json`, creating it if it does not exist.
/// The caller is responsible for the returned memory.
#[no_mangle]
pub extern "C" fn EngineOptions_create(
    stracciatella_home: *const c_char,
    args: *const *const c_char,
    length: size_t,
) -> *mut EngineOptions {
    let stracciatella_home = path_buf_from_c_str_or_panic(unsafe_c_str(stracciatella_home));
    let args: Vec<String> = unsafe_slice(args, length)
        .iter()
        .map(|&x| str_from_c_str_or_panic(unsafe_c_str(x)).to_owned())
        .collect();

    match EngineOptions::from_home_and_args(&stracciatella_home, &args) {
        Ok(engine_options) => {
            if engine_options.show_help {
                print!("{}", Cli::usage());
            }
            no_rust_error();
            into_ptr(engine_options)
        }
        Err(msg) => {
            remember_rust_error(format!("EngineOptions_create: {}", msg));
            ptr::null_mut()
        }
    }
}

/// Creates empty engine options
#[no_mangle]
pub extern "C" fn EngineOptions_default() -> *mut EngineOptions {
    into_ptr(EngineOptions::default())
}

/// Writes `EngineOptions` to `(stracciatella_home)/ja2.json`.
/// Returns true on success, false otherwise.
#[no_mangle]
pub extern "C" fn EngineOptions_write(ptr: *mut EngineOptions) -> bool {
    let engine_options = unsafe_mut(ptr);
    let ja2_json = Ja2Json::from_stracciatella_home(&engine_options.stracciatella_home);
    ja2_json.write(&engine_options).is_ok()
}

/// Deletes `EngineOptions`.
#[no_mangle]
pub extern "C" fn EngineOptions_destroy(ptr: *mut EngineOptions) {
    let _drop_me = from_ptr(ptr);
}

/// Gets the `EngineOptions.stracciatella_home` path.
/// The caller is responsible for the returned memory.
#[no_mangle]
pub extern "C" fn EngineOptions_getStracciatellaHome() -> *mut c_char {
    let stracciatella_home = find_stracciatella_home();

    forget_rust_error();
    match stracciatella_home {
        Ok(stracciatella_home) => {
            let stracciatella_home = c_string_from_path_or_panic(&stracciatella_home);
            stracciatella_home.into_raw()
        }
        Err(e) => {
            remember_rust_error(format!("EngineOptions_getStracciatellaHome: {:?}", e));
            std::ptr::null_mut()
        }
    }
}

/// Gets the `EngineOptions.vanilla_game_dir` path.
/// The caller is responsible for the returned memory.
#[no_mangle]
pub extern "C" fn EngineOptions_getVanillaGameDir(ptr: *const EngineOptions) -> *mut c_char {
    let engine_options = unsafe_ref(ptr);
    let vanilla_game_dir = c_string_from_path_or_panic(&engine_options.vanilla_game_dir);
    vanilla_game_dir.into_raw()
}

/// Gets the `EngineOptions.assets_dir` path.
/// The caller is responsible for the returned memory.
#[no_mangle]
pub extern "C" fn EngineOptions_getAssetsDir(ptr: *const EngineOptions) -> *mut c_char {
    let engine_options = unsafe_ref(ptr);
    let vanilla_game_dir = c_string_from_path_or_panic(&engine_options.assets_dir);
    vanilla_game_dir.into_raw()
}

/// Sets the `EngineOptions.vanilla_game_dir` path.
#[no_mangle]
pub extern "C" fn EngineOptions_setVanillaGameDir(
    ptr: *mut EngineOptions,
    game_dir_ptr: *const c_char,
) {
    let engine_options = unsafe_mut(ptr);
    let vanilla_game_dir = path_buf_from_c_str_or_panic(unsafe_c_str(game_dir_ptr));
    engine_options.vanilla_game_dir = vanilla_game_dir;
}

/// Gets the length of `EngineOptions.mods`.
#[no_mangle]
pub extern "C" fn EngineOptions_getModsLength(ptr: *const EngineOptions) -> u32 {
    let engine_options = unsafe_ref(ptr);
    engine_options.mods.len() as u32
}

/// Gets the target index of `EngineOptions.mods`.
/// The caller is responsible for the returned memory.
#[no_mangle]
pub extern "C" fn EngineOptions_getMod(ptr: *const EngineOptions, index: u32) -> *mut c_char {
    let engine_options = unsafe_ref(ptr);
    match engine_options.mods.get(index as usize) {
        Some(str_mod) => {
            let c_str_mod = c_string_from_str(&str_mod);
            c_str_mod.into_raw()
        }
        None => {
            let len = engine_options.mods.len();
            panic!("Invalid mod index {}, len = {}", index, len);
        }
    }
}

/// Clears `EngineOptions.mods`.
#[no_mangle]
pub extern "C" fn EngineOptions_clearMods(ptr: *mut EngineOptions) {
    let engine_options = unsafe_mut(ptr);
    engine_options.mods.clear();
}

/// Adds a mod to `EngineOptions.mods`.
#[no_mangle]
pub extern "C" fn EngineOptions_pushMod(ptr: *mut EngineOptions, name: *const c_char) {
    let engine_options = unsafe_mut(ptr);
    let name = str_from_c_str_or_panic(unsafe_c_str(name)).to_owned();
    engine_options.mods.push(name);
}

/// Gets the width of `EngineOptions.resolution`.
#[no_mangle]
pub extern "C" fn EngineOptions_getResolutionX(ptr: *const EngineOptions) -> u16 {
    let engine_options = unsafe_ref(ptr);
    engine_options.resolution.0
}

/// Gets the height of `EngineOptions.resolution`.
#[no_mangle]
pub extern "C" fn EngineOptions_getResolutionY(ptr: *const EngineOptions) -> u16 {
    let engine_options = unsafe_ref(ptr);
    engine_options.resolution.1
}

/// Sets `EngineOptions.resolution`.
#[no_mangle]
pub extern "C" fn EngineOptions_setResolution(ptr: *mut EngineOptions, x: u16, y: u16) {
    let engine_options = unsafe_mut(ptr);
    engine_options.resolution = Resolution(x, y);
}

/// Gets `EngineOptions.brightness`.
#[no_mangle]
pub extern "C" fn EngineOptions_getBrightness(ptr: *const EngineOptions) -> f32 {
    let engine_options = unsafe_ref(ptr);
    engine_options.brightness
}

/// Sets `EngineOptions.brightness`.
#[no_mangle]
pub extern "C" fn EngineOptions_setBrightness(ptr: *mut EngineOptions, brightness: f32) {
    let engine_options = unsafe_mut(ptr);
    engine_options.brightness = brightness
}

/// Gets `EngineOptions.resource_version`.
#[no_mangle]
pub extern "C" fn EngineOptions_getResourceVersion(ptr: *const EngineOptions) -> VanillaVersion {
    let engine_options = unsafe_ref(ptr);
    engine_options.resource_version
}

/// Sets `EngineOptions.resource_version`.
#[no_mangle]
pub extern "C" fn EngineOptions_setResourceVersion(ptr: *mut EngineOptions, res: VanillaVersion) {
    let engine_options = unsafe_mut(ptr);
    engine_options.resource_version = res;
}

/// Gets `EngineOptions.run_unittests`.
#[no_mangle]
pub extern "C" fn EngineOptions_shouldRunUnittests(ptr: *const EngineOptions) -> bool {
    let engine_options = unsafe_ref(ptr);
    engine_options.run_unittests
}

/// Gets `EngineOptions.show_help`.
#[no_mangle]
pub extern "C" fn EngineOptions_shouldShowHelp(ptr: *const EngineOptions) -> bool {
    let engine_options = unsafe_ref(ptr);
    engine_options.show_help
}

/// Gets `EngineOptions.run_editor`.
#[no_mangle]
pub extern "C" fn EngineOptions_shouldRunEditor(ptr: *const EngineOptions) -> bool {
    let engine_options = unsafe_ref(ptr);
    engine_options.run_editor
}

/// Gets `EngineOptions.start_in_fullscreen`.
#[no_mangle]
pub extern "C" fn EngineOptions_shouldStartInFullscreen(ptr: *const EngineOptions) -> bool {
    let engine_options = unsafe_ref(ptr);
    engine_options.start_in_fullscreen
}

/// Sets `EngineOptions.start_in_fullscreen`.
#[no_mangle]
pub extern "C" fn EngineOptions_setStartInFullscreen(ptr: *mut EngineOptions, val: bool) {
    let engine_options = unsafe_mut(ptr);
    engine_options.start_in_fullscreen = val
}

/// Gets `EngineOptions.scaling_quality`.
#[no_mangle]
pub extern "C" fn EngineOptions_getScalingQuality(ptr: *const EngineOptions) -> ScalingQuality {
    let engine_options = unsafe_ref(ptr);
    engine_options.scaling_quality
}

/// Sets `EngineOptions.scaling_quality`.
#[no_mangle]
pub extern "C" fn EngineOptions_setScalingQuality(
    ptr: *mut EngineOptions,
    scaling_quality: ScalingQuality,
) {
    let engine_options = unsafe_mut(ptr);
    engine_options.scaling_quality = scaling_quality
}

/// Gets `EngineOptions.start_in_window`.
#[no_mangle]
pub extern "C" fn EngineOptions_shouldStartInWindow(ptr: *const EngineOptions) -> bool {
    let engine_options = unsafe_ref(ptr);
    engine_options.start_in_window
}

/// Gets `EngineOptions.start_in_debug_mode`.
#[no_mangle]
pub extern "C" fn EngineOptions_shouldStartInDebugMode(ptr: *const EngineOptions) -> bool {
    let engine_options = unsafe_ref(ptr);
    engine_options.start_in_debug_mode
}

/// Gets `EngineOptions.start_without_sound`.
#[no_mangle]
pub extern "C" fn EngineOptions_shouldStartWithoutSound(ptr: *const EngineOptions) -> bool {
    let engine_options = unsafe_ref(ptr);
    engine_options.start_without_sound
}

/// Sets `EngineOptions.start_without_sound`.
#[no_mangle]
pub extern "C" fn EngineOptions_setStartWithoutSound(ptr: *mut EngineOptions, val: bool) {
    let engine_options = unsafe_mut(ptr);
    engine_options.start_without_sound = val
}

/// Gets the string representation of the `ScalingQuality` value.
/// The caller is responsible for the returned memory.
#[no_mangle]
pub extern "C" fn ScalingQuality_toString(quality: ScalingQuality) -> *mut c_char {
    let c_string = c_string_from_str(&quality.to_string());
    c_string.into_raw()
}

/// Gets the string represntation of the `VanillaVersion` value.
/// The caller is responsible for the returned memory.
#[no_mangle]
pub extern "C" fn VanillaVersion_toString(version: VanillaVersion) -> *mut c_char {
    let c_string = c_string_from_str(&version.to_string());
    c_string.into_raw()
}

#[cfg(test)]
mod tests {
    use std::fs;

    use stracciatella::config::{EngineOptions, Resolution};
    use stracciatella::parse_json_config;
    use tempfile::TempDir;

    use crate::c::common::*;
    use crate::c::config::*;
    use crate::c::misc::CString_destroy;

    fn write_temp_folder_with_ja2_json(contents: &[u8]) -> TempDir {
        let dir = TempDir::new().unwrap();
        let ja2_home_dir = dir.path().join(".ja2");
        let file_path = ja2_home_dir.join("ja2.json");

        fs::create_dir(ja2_home_dir).unwrap();
        fs::write(file_path, contents).unwrap();

        dir
    }

    #[test]
    fn write_engine_options_should_write_a_json_file_that_can_be_serialized_again() {
        let mut engine_options = EngineOptions::default();
        let temp_dir = write_temp_folder_with_ja2_json(b"Invalid JSON");
        let stracciatella_home = temp_dir.path().join(".ja2");

        engine_options.stracciatella_home = stracciatella_home.clone();
        engine_options.resolution = Resolution(100, 100);

        assert_eq!(EngineOptions_write(&mut engine_options), true);

        let got_engine_options = parse_json_config(&stracciatella_home).unwrap();

        assert_eq!(got_engine_options.resolution, engine_options.resolution);
    }

    #[test]
    fn write_engine_options_should_write_a_pretty_json_file() {
        let mut engine_options = EngineOptions::default();
        let temp_dir = write_temp_folder_with_ja2_json(b"Invalid JSON");
        let stracciatella_home = temp_dir.path().join(".ja2");
        let stracciatella_json = temp_dir.path().join(".ja2/ja2.json");

        engine_options.stracciatella_home = stracciatella_home;
        engine_options.resolution = Resolution(100, 100);

        EngineOptions_write(&mut engine_options);

        let config_file_contents = fs::read_to_string(stracciatella_json).unwrap();

        assert_eq!(
            config_file_contents,
            r##"{
  "game_dir": "",
  "mods": [],
  "res": "100x100",
  "brightness": 1.0,
  "resversion": "ENGLISH",
  "fullscreen": false,
  "scaling": "PERFECT",
  "debug": false,
  "nosound": false
}"##
        );
    }

    #[test]
    fn vanilla_version_to_string_should_return_the_correct_resource_version_string() {
        macro_rules! t {
            ($version:expr, $expected:expr) => {
                let got = VanillaVersion_toString($version);
                assert_eq!(str_from_c_str_or_panic(unsafe_c_str(got)), $expected);
                CString_destroy(got);
            };
        }
        t!(VanillaVersion::DUTCH, "Dutch");
        t!(VanillaVersion::ENGLISH, "English");
        t!(VanillaVersion::FRENCH, "French");
        t!(VanillaVersion::GERMAN, "German");
        t!(VanillaVersion::ITALIAN, "Italian");
        t!(VanillaVersion::POLISH, "Polish");
        t!(VanillaVersion::RUSSIAN, "Russian");
        t!(VanillaVersion::RUSSIAN_GOLD, "Russian (Gold)");
    }
}
