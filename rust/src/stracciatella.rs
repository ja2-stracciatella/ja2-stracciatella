#![crate_type = "lib"]

extern crate getopts;
extern crate libc;

use getopts::Options;
use libc::{size_t, c_char};
use std::slice;
use std::ffi::{CStr, CString};
use std::str;
use std::env;

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

pub struct CommandLineArgs {
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

pub fn get_options() -> Options {
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

fn parse_args(args: Vec<String>) -> Result<CommandLineArgs, String> {
    let opts = get_options();

    match opts.parse(&args[1..]) {
        Ok(m) => {
            let mut command_line_args = CommandLineArgs {
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
            };

            if m.free.len() > 0 {
                return Err(format!("Unknown arguments: '{}'.", m.free.join(" ")));
            }

            if m.opt_strs("mod").len() > 0 {
                command_line_args.mods = m.opt_strs("mod");
            }

            match m.opt_str("res") {
                Some(s) => {
                    let mut resolutions = s.split("x").filter_map(|r_str| r_str.parse::<u16>().ok());
                    let resolution_x = resolutions.next();
                    let resolution_y = resolutions.next();

                    match (resolution_x, resolution_y) {
                        (Some(x), Some(y)) => {
                            command_line_args.resolution_x = x;
                            command_line_args.resolution_y = y;
                        }
                        (None, _) | (_, None) => {
                            return Err(String::from("Resolution argument incorrect format, should be WIDTHxHEIGHT."));
                        }
                    }
                },
                None => {}
            }

            match m.opt_str("resversion") {
                Some(s) => {
                    match get_res_version(s.as_ref()) {
                        Some(resource_version) => {
                            command_line_args.resource_version = resource_version
                        },
                        None => return Err(format!("Unknown resource version: '{}'.", s))
                    }
                },
                None => {}
            }

            if m.opt_present("unittests") {
                command_line_args.run_unittests = true;
            }

            if m.opt_present("editor") {
                command_line_args.run_editor = true;
            }

            if m.opt_present("fullscreen") {
                command_line_args.start_in_fullscreen = true;
            }

            if m.opt_present("nosound") {
                command_line_args.start_without_sound = true;
            }

            if m.opt_present("window") {
                command_line_args.start_in_window = true;
            }

            if m.opt_present("debug") {
                command_line_args.start_in_debug_mode = true;
            }

            return Ok(command_line_args);
        }
        Err(f) => {
            return Err(f.to_string());
        }
    }
}

#[no_mangle]
pub fn create_command_line_args(array: *const *const c_char, length: size_t) -> *mut CommandLineArgs {
    let values = unsafe { slice::from_raw_parts(array, length as usize) };
    let args: Vec<String> = values.iter()
        .map(|&p| unsafe { CStr::from_ptr(p) })  // iterator of &CStr
        .map(|cs| cs.to_bytes())                 // iterator of &[u8]
        .map(|bs| String::from(str::from_utf8(bs).unwrap()))   // iterator of &str
        .collect();

    match env::home_dir() {
        Some(path) => println!("Home Dir: {}", path.display()),
        None => println!("Impossible to get your home dir!"),
    }

    println!("Args: {:?}", args);
    return match parse_args(args) {
        Ok(parsed_args) => Box::into_raw(Box::new(parsed_args)),
        Err(msg) => panic!(msg)
    };
}

#[no_mangle]
pub fn free_command_line_args(ptr: *mut CommandLineArgs) {
    if ptr.is_null() { return }
    unsafe { Box::from_raw(ptr); }
}

#[no_mangle]
pub extern fn get_number_of_mods(ptr: *const CommandLineArgs) -> u32 {
    let command_line_args = unsafe { assert!(!ptr.is_null()); &*ptr };
    return command_line_args.mods.len() as u32
}

#[no_mangle]
pub extern fn get_mod(ptr: *const CommandLineArgs, index: u32) -> *mut c_char {
    let command_line_args = unsafe { assert!(!ptr.is_null()); &*ptr };
    let str_mod = match command_line_args.mods.get(index as usize) {
        Some(m) => m,
        None => panic!("Invalid mod index for game options {}", index)
    };
    let c_str_mod = CString::new(str_mod.clone()).unwrap();
    c_str_mod.into_raw()
}

#[no_mangle]
pub extern fn get_resolution_x(ptr: *const CommandLineArgs) -> u16 {
    let command_line_args = unsafe { assert!(!ptr.is_null()); &*ptr };
    return command_line_args.resolution_x
}

#[no_mangle]
pub extern fn get_resolution_y(ptr: *const CommandLineArgs) -> u16 {
    let command_line_args = unsafe { assert!(!ptr.is_null()); &*ptr };
    return command_line_args.resolution_y
}

#[no_mangle]
pub extern fn get_resource_version(ptr: *const CommandLineArgs) -> ResourceVersion {
    let command_line_args = unsafe { assert!(!ptr.is_null()); &*ptr };
    command_line_args.resource_version
}

#[no_mangle]
pub fn should_run_unittests(ptr: *const CommandLineArgs) -> bool {
    let command_line_args = unsafe { assert!(!ptr.is_null()); &*ptr };
    command_line_args.run_unittests
}

#[no_mangle]
pub fn should_run_editor(ptr: *const CommandLineArgs) -> bool {
    let command_line_args = unsafe { assert!(!ptr.is_null()); &*ptr };
    command_line_args.run_editor
}

#[no_mangle]
pub fn should_start_in_fullscreen(ptr: *const CommandLineArgs) -> bool {
    let command_line_args = unsafe { assert!(!ptr.is_null()); &*ptr };
    command_line_args.start_in_fullscreen
}

#[no_mangle]
pub fn should_start_in_window(ptr: *const CommandLineArgs) -> bool {
    let command_line_args = unsafe { assert!(!ptr.is_null()); &*ptr };
    command_line_args.start_in_window
}

#[no_mangle]
pub fn should_start_in_debug_mode(ptr: *const CommandLineArgs) -> bool {
    let command_line_args = unsafe { assert!(!ptr.is_null()); &*ptr };
    command_line_args.start_in_debug_mode
}

#[no_mangle]
pub fn should_start_without_sound(ptr: *const CommandLineArgs) -> bool {
    let command_line_args = unsafe { assert!(!ptr.is_null()); &*ptr };
    command_line_args.start_without_sound
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
    #[test]
    fn it_should_abort_on_unknown_arguments() {
        let input = vec!(String::from("ja2"), String::from("testunknown"));
        assert_eq!(super::parse_args(input).err().unwrap(), "Unknown arguments: 'testunknown'.");
    }

    #[test]
    fn it_should_abort_on_unknown_switch() {
        let input = vec!(String::from("ja2"), String::from("--testunknown"));
        assert_eq!(super::parse_args(input).err().unwrap(), "Unrecognized option: 'testunknown'.");
    }

    #[test]
    fn it_should_have_correct_fullscreen_default_value() {
        let input = vec!(String::from("ja2"));
        assert!(!super::should_start_in_fullscreen(&super::parse_args(input).unwrap()));
    }

    #[test]
    fn it_should_be_able_to_change_fullscreen_value() {
        let input = vec!(String::from("ja2"), String::from("--fullscreen"));
        assert!(super::should_start_in_fullscreen(&super::parse_args(input).unwrap()));
    }

    #[test]
    fn it_should_continue_with_multiple_known_switches() {
        let input = vec!(String::from("ja2"), String::from("--debug"), String::from("--mod"), String::from("a"), String::from("--mod"), String::from("b"));
        let args = super::parse_args(input).unwrap();

        assert!(super::should_start_in_debug_mode(&args));
        assert!(super::get_number_of_mods(&args) == 2);
    }

    #[test]
    fn it_should_fail_with_unknown_resversion() {
        let input = vec!(String::from("ja2"), String::from("--resversion"), String::from("TESTUNKNOWN"));
        assert_eq!(super::parse_args(input).err().unwrap(), "Unknown resource version: 'TESTUNKNOWN'.");
    }

    #[test]
    fn it_should_return_the_correct_resversion_for_russian() {
        let input = vec!(String::from("ja2"), String::from("--resversion"), String::from("RUSSIAN"));
        let args = super::parse_args(input).unwrap();
        assert!(super::get_resource_version(&args) == super::ResourceVersion::RUSSIAN);
    }

    #[test]
    fn it_should_return_the_correct_resversion_for_italian() {
        let input = vec!(String::from("ja2"), String::from("--resversion"), String::from("ITALIAN"));
        let args = super::parse_args(input).unwrap();
        assert!(super::get_resource_version(&args) == super::ResourceVersion::ITALIAN);
    }

    #[test]
    fn it_should_return_the_correct_resolution() {
        let input = vec!(String::from("ja2"), String::from("--res"), String::from("1120x960"));
        let args = super::parse_args(input).unwrap();
        assert_eq!(super::get_resolution_x(&args), 1120);
        assert_eq!(super::get_resolution_y(&args), 960);
    }

    #[test]
    fn it_should_fail_with_bad_resolution() {
        let input = vec!(String::from("ja2"), String::from("--res"), String::from("1120xaaa"));
        assert_eq!(super::parse_args(input).err().unwrap(), "Resolution argument incorrect format, should be WIDTHxHEIGHT.");
    }
}
