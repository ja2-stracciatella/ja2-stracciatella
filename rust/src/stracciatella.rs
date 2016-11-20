#![crate_type = "lib"]

extern crate getopts;
extern crate libc;
#[cfg(windows)]
extern crate winapi;
#[cfg(windows)]
extern crate user32;
#[cfg(windows)]
extern crate shell32;

use getopts::Options;
use libc::{size_t, c_char};
use std::slice;
use std::ffi::{CStr, CString};
use std::str;
use std::path::PathBuf;
use std::default::Default;

#[derive(Debug, PartialEq, Copy, Clone)]
#[repr(C)]
pub enum ResourceVersion {
    DUTCH,
    ENGLISH,
    FRENCH,
    GERMAN,
    ITALIAN,
    POLISH,
    RUSSIAN,
    RUSSIAN_GOLD,
}

pub struct EngineOptions {
    stracciatella_home: PathBuf,
    mods: Vec<String>,
    resolution_x: u16,
    resolution_y: u16,
    resource_version: ResourceVersion,
    run_unittests: bool,
    run_editor: bool,
    start_in_fullscreen: bool,
    start_in_window: bool,
    start_in_debug_mode: bool,
    start_without_sound: bool,
}

impl Default for EngineOptions {
    fn default() -> EngineOptions {
        EngineOptions {
            stracciatella_home: PathBuf::from(""),
            mods: vec!(),
            resolution_x: 640,
            resolution_y: 480,
            resource_version: ResourceVersion::ENGLISH,
            run_unittests: false,
            run_editor: false,
            start_in_fullscreen: false,
            start_in_window: true,
            start_in_debug_mode: false,
            start_without_sound: false,
        }
    }
}

pub fn get_command_line_options() -> Options {
    let mut opts = Options::new();

    opts.long_only(true);

    opts.optmulti(
        "m",
        "mod",
        "Start one of the game modifications. MOD_NAME is the name of modification, e.g. 'from-russia-with-love. See mods folder for possible options'.",
        "MOD_NAME"
    );
    opts.optopt(
        "r",
        "res",
        "Screen resolution, e.g. 800x600. Default value is 640x480",
        "WIDTHxHEIGHT"
    );
    opts.optopt(
        "l",
        "resversion",
        "Version of the game resources. Possible values: DUTCH, ENGLISH, FRENCH, GERMAN, ITALIAN, POLISH, RUSSIAN, RUSSIAN_GOLD. Default value is ENGLISH. RUSSIAN is for BUKA Agonia Vlasty release. RUSSIAN_GOLD is for Gold release",
        "RUSSIAN_GOLD"
    );
    opts.optflag(
        "u",
        "unittests",
        "Perform unit tests. E.g. ja2.exe -unittests --gtest_output=\"xml:report.xml\" --gtest_repeat=2");
    opts.optflag(
        "e",
        "editor",
        "Start the map editor (Editor.slf is required)"
    );
    opts.optflag(
        "f",
        "fullscreen",
        "Start the game in the fullscreen mode"
    );
    opts.optflag(
        "n",
        "nosound",
        "Turn the sound and music off"
    );
    opts.optflag(
        "w",
        "window",
        "Start the game in a window"
    );
    opts.optflag(
        "d",
        "debug",
        "Enable Debug Mode"
    );
    opts.optflag(
        "h",
        "help",
        "print this help menu"
    );

    return opts;
}

fn get_res_version(res_version_str: &str) -> Option<ResourceVersion> {
    match res_version_str {
        "DUTCH" => Some(ResourceVersion::RUSSIAN),
        "ENGLISH" => Some(ResourceVersion::ENGLISH),
        "FRENCH" => Some(ResourceVersion::FRENCH),
        "GERMAN" => Some(ResourceVersion::GERMAN),
        "ITALIAN" => Some(ResourceVersion::ITALIAN),
        "POLISH" => Some(ResourceVersion::POLISH),
        "RUSSIAN" => Some(ResourceVersion::RUSSIAN),
        "RUSSIAN_GOLD" => Some(ResourceVersion::RUSSIAN_GOLD),
        _ => None
    }
}

fn parse_args(engine_options: &mut EngineOptions, args: Vec<String>) -> Option<String> {
    let opts = get_command_line_options();

    match opts.parse(&args[1..]) {
        Ok(m) => {
            if m.free.len() > 0 {
                return Some(format!("Unknown arguments: '{}'.", m.free.join(" ")));
            }

            if m.opt_strs("mod").len() > 0 {
                engine_options.mods = m.opt_strs("mod");
            }

            match m.opt_str("res") {
                Some(s) => {
                    let mut resolutions = s.split("x").filter_map(|r_str| r_str.parse::<u16>().ok());
                    let resolution_x = resolutions.next();
                    let resolution_y = resolutions.next();

                    match (resolution_x, resolution_y) {
                        (Some(x), Some(y)) => {
                            engine_options.resolution_x = x;
                            engine_options.resolution_y = y;
                        }
                        (None, _) | (_, None) => {
                            return Some(String::from("Resolution argument incorrect format, should be WIDTHxHEIGHT."));
                        }
                    }
                },
                None => {}
            }

            match m.opt_str("resversion") {
                Some(s) => {
                    match get_res_version(s.as_ref()) {
                        Some(resource_version) => {
                            engine_options.resource_version = resource_version
                        },
                        None => return Some(format!("Unknown resource version: '{}'.", s))
                    }
                },
                None => {}
            }

            if m.opt_present("unittests") {
                engine_options.run_unittests = true;
            }

            if m.opt_present("editor") {
                engine_options.run_editor = true;
            }

            if m.opt_present("fullscreen") {
                engine_options.start_in_fullscreen = true;
            }

            if m.opt_present("nosound") {
                engine_options.start_without_sound = true;
            }

            if m.opt_present("window") {
                engine_options.start_in_window = true;
            }

            if m.opt_present("debug") {
                engine_options.start_in_debug_mode = true;
            }

            return None;
        }
        Err(f) => {
            return Some(f.to_string());
        }
    }
}

#[cfg(not(windows))]
pub fn find_stracciatella_home() -> Result<PathBuf, String> {
    use std::env;

    match env::home_dir() {
        Some(mut path) => {
            path.push(".ja2");
            return Ok(path);
        },
        None => Err(String::from("Failed to find home directory")),
    }
}

#[cfg(windows)]
pub fn find_stracciatella_home() -> Result<PathBuf, String> {
    use std::ptr::null_mut;
    use shell32::SHGetFolderPathA;
    use winapi::shlobj::{CSIDL_PERSONAL, CSIDL_FLAG_CREATE};
    use winapi::minwindef::MAX_PATH;
    use std::ffi::CStr;

    let mut home: [u8; MAX_PATH] = [0; MAX_PATH];
    unsafe {
        return match SHGetFolderPathA(null_mut(), CSIDL_PERSONAL | CSIDL_FLAG_CREATE, null_mut(), 0, home.as_mut_ptr() as *mut i8) {
            0 => {
                let home_cstr = CStr::from_ptr(home.as_ptr() as *const i8);
                return match home_cstr.to_str() {
                    Ok(s) => {
                        let mut buf = PathBuf::from(s);
                        buf.push("JA2");
                        return Ok(buf);
                    },
                    Err(e) => Err(format!("Error decoding documents folder string: {}", e))
                }
            },
            i => Err(format!("Cannot get documents folder error: {}", i))
        };
    }
}

#[no_mangle]
pub fn create_engine_options(array: *const *const c_char, length: size_t) -> *mut EngineOptions {
    let mut engine_options: EngineOptions = Default::default();
    let values = unsafe { slice::from_raw_parts(array, length as usize) };
    let args: Vec<String> = values.iter()
        .map(|&p| unsafe { CStr::from_ptr(p) })  // iterator of &CStr
        .map(|cs| cs.to_bytes())                 // iterator of &[u8]
        .map(|bs| String::from(str::from_utf8(bs).unwrap()))   // iterator of &str
        .collect();



    return match find_stracciatella_home() {
        Ok(home_dir) => {
            engine_options.stracciatella_home = home_dir;

            let res_parse_args = parse_args(&mut engine_options, args);

            return match res_parse_args {
                None => Box::into_raw(Box::new(engine_options)),
                Some(msg) => panic!(msg)
            };
        },
        Err(msg) => panic!(msg)
    }

}

#[no_mangle]
pub fn free_engine_options(ptr: *mut EngineOptions) {
    if ptr.is_null() { return }
    unsafe { Box::from_raw(ptr); }
}

#[no_mangle]
pub extern fn get_number_of_mods(ptr: *const EngineOptions) -> u32 {
    let engine_options = unsafe { assert!(!ptr.is_null()); &*ptr };
    return engine_options.mods.len() as u32
}

#[no_mangle]
pub extern fn get_mod(ptr: *const EngineOptions, index: u32) -> *mut c_char {
    let engine_options = unsafe { assert!(!ptr.is_null()); &*ptr };
    let str_mod = match engine_options.mods.get(index as usize) {
        Some(m) => m,
        None => panic!("Invalid mod index for game options {}", index)
    };
    let c_str_mod = CString::new(str_mod.clone()).unwrap();
    c_str_mod.into_raw()
}

#[no_mangle]
pub extern fn get_resolution_x(ptr: *const EngineOptions) -> u16 {
    let engine_options = unsafe { assert!(!ptr.is_null()); &*ptr };
    return engine_options.resolution_x
}

#[no_mangle]
pub extern fn get_resolution_y(ptr: *const EngineOptions) -> u16 {
    let engine_options = unsafe { assert!(!ptr.is_null()); &*ptr };
    return engine_options.resolution_y
}

#[no_mangle]
pub extern fn get_resource_version(ptr: *const EngineOptions) -> ResourceVersion {
    let engine_options = unsafe { assert!(!ptr.is_null()); &*ptr };
    engine_options.resource_version
}

#[no_mangle]
pub fn should_run_unittests(ptr: *const EngineOptions) -> bool {
    let engine_options = unsafe { assert!(!ptr.is_null()); &*ptr };
    engine_options.run_unittests
}

#[no_mangle]
pub fn should_run_editor(ptr: *const EngineOptions) -> bool {
    let engine_options = unsafe { assert!(!ptr.is_null()); &*ptr };
    engine_options.run_editor
}

#[no_mangle]
pub fn should_start_in_fullscreen(ptr: *const EngineOptions) -> bool {
    let engine_options = unsafe { assert!(!ptr.is_null()); &*ptr };
    engine_options.start_in_fullscreen
}

#[no_mangle]
pub fn should_start_in_window(ptr: *const EngineOptions) -> bool {
    let engine_options = unsafe { assert!(!ptr.is_null()); &*ptr };
    engine_options.start_in_window
}

#[no_mangle]
pub fn should_start_in_debug_mode(ptr: *const EngineOptions) -> bool {
    let engine_options = unsafe { assert!(!ptr.is_null()); &*ptr };
    engine_options.start_in_debug_mode
}

#[no_mangle]
pub fn should_start_without_sound(ptr: *const EngineOptions) -> bool {
    let engine_options = unsafe { assert!(!ptr.is_null()); &*ptr };
    engine_options.start_without_sound
}

#[no_mangle]
pub fn free_rust_string(s: *mut c_char) {
    unsafe {
        if s.is_null() { return }
        CString::from_raw(s)
    };
}


#[cfg(test)]
mod tests {
    extern crate regex;

    #[test]
    fn it_should_abort_on_unknown_arguments() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = vec!(String::from("ja2"), String::from("testunknown"));
        assert_eq!(super::parse_args(&mut engine_options, input).unwrap(), "Unknown arguments: 'testunknown'.");
    }

    #[test]
    fn it_should_abort_on_unknown_switch() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = vec!(String::from("ja2"), String::from("--testunknown"));
        assert_eq!(super::parse_args(&mut engine_options, input).unwrap(), "Unrecognized option: 'testunknown'.");
    }

    #[test]
    fn it_should_have_correct_fullscreen_default_value() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = vec!(String::from("ja2"));
        assert!(super::parse_args(&mut engine_options, input).is_none());
        assert!(!super::should_start_in_fullscreen(&engine_options));
    }

    #[test]
    fn it_should_be_able_to_change_fullscreen_value() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = vec!(String::from("ja2"), String::from("--fullscreen"));
        assert!(super::parse_args(&mut engine_options, input).is_none());
        assert!(super::should_start_in_fullscreen(&engine_options));
    }

    #[test]
    fn it_should_continue_with_multiple_known_switches() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = vec!(String::from("ja2"), String::from("--debug"), String::from("--mod"), String::from("a"), String::from("--mod"), String::from("b"));
        assert!(super::parse_args(&mut engine_options, input).is_none());
        assert!(super::should_start_in_debug_mode(&engine_options));
        assert!(super::get_number_of_mods(&engine_options) == 2);
    }

    #[test]
    fn it_should_fail_with_unknown_resversion() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = vec!(String::from("ja2"), String::from("--resversion"), String::from("TESTUNKNOWN"));
        assert_eq!(super::parse_args(&mut engine_options, input).unwrap(), "Unknown resource version: 'TESTUNKNOWN'.");
    }

    #[test]
    fn it_should_return_the_correct_resversion_for_russian() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = vec!(String::from("ja2"), String::from("--resversion"), String::from("RUSSIAN"));
        assert!(super::parse_args(&mut engine_options, input).is_none());
        assert!(super::get_resource_version(&engine_options) == super::ResourceVersion::RUSSIAN);
    }

    #[test]
    fn it_should_return_the_correct_resversion_for_italian() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = vec!(String::from("ja2"), String::from("--resversion"), String::from("ITALIAN"));
        assert!(super::parse_args(&mut engine_options, input).is_none());
        assert!(super::get_resource_version(&engine_options) == super::ResourceVersion::ITALIAN);
    }

    #[test]
    fn it_should_return_the_correct_resolution() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = vec!(String::from("ja2"), String::from("--res"), String::from("1120x960"));
        assert!(super::parse_args(&mut engine_options, input).is_none());
        assert_eq!(super::get_resolution_x(&engine_options), 1120);
        assert_eq!(super::get_resolution_y(&engine_options), 960);
    }

    #[test]
    #[cfg(not(windows))]
    fn it_should_find_the_correct_stracciatella_home_path_on_unixlike() {
        use std::env;

        assert_eq!(super::find_stracciatella_home().unwrap().to_str().unwrap(), format!("{}/.ja2", env::var("HOME").unwrap()));
    }

    #[test]
    #[cfg(windows)]
    fn it_should_find_the_correct_stracciatella_home_path_on_windows() {
        use self::regex::Regex;

        let result = super::find_stracciatella_home().unwrap();
        let regex = Regex::new(r"^[A-Z]:\\(.*)+\\JA2").unwrap();
        assert!(regex.is_match(result.to_str().unwrap()));
    }
}
