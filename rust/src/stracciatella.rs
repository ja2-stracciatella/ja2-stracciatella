#![crate_type = "lib"]
//https://github.com/rust-lang/rfcs/pull/2585
#![allow(unused_unsafe)]

extern crate getopts;
extern crate libc;
extern crate serde;
extern crate serde_json;
extern crate serde_derive;
extern crate dirs;

use std::env;
use std::str;
use std::path::{PathBuf};
use std::default::Default;

pub mod config;
pub mod guess;
pub mod json;
pub mod logger;
pub mod res;
pub mod file_formats;
pub mod librarydb;
pub mod c;
pub mod unicode;

pub use crate::config::ScalingQuality;
pub use crate::config::VanillaVersion;
use crate::config::Resolution;
use crate::config::Ja2Json;
use crate::config::Cli;
use crate::config::EngineOptions;
use crate::config::find_stracciatella_home;

fn parse_args(engine_options: &mut EngineOptions, args: &[String]) -> Option<String> {
    let cli = Cli::from_args(args);
    cli.apply_to_engine_options(engine_options).err()
}

pub fn ensure_json_config_existence(stracciatella_home: &PathBuf) -> Result<(), String> {
    let ja2_json = Ja2Json::from_stracciatella_home(stracciatella_home);
    ja2_json.ensure_existence()
}


pub fn parse_json_config(stracciatella_home: &PathBuf) -> Result<EngineOptions, String> {
    let mut engine_options = EngineOptions::default();
    let ja2_json = Ja2Json::from_stracciatella_home(stracciatella_home);

    ja2_json.apply_to_engine_options(&mut engine_options)?;

    Ok(engine_options)
}

/// Returns the path to the assets directory.
/// It contains mods and externalized subdirectories.
fn get_assets_dir() -> PathBuf {
    let mut path = PathBuf::new();
    let extra_data_dir = option_env!("EXTRA_DATA_DIR");
    if extra_data_dir.is_some() && extra_data_dir.unwrap().len() > 0 {
        // use dir defined at compile time
        path.push(extra_data_dir.unwrap());
    } else if let Ok(exe) = env::current_exe() {
        if let Some(dir) = exe.parent() {
            // use the directory of the executable
            path.push(dir);
        }
    }
    path
}

#[cfg(test)]
mod tests {
    extern crate regex;
    extern crate tempdir;

    use std::path::{PathBuf};
    use std::str;
    use std::ffi::{CStr, CString};
    use std::fs;
    use std::fs::File;
    use std::io::prelude::*;
    use std::env;

    use crate::c::config::*;

    macro_rules! assert_chars_eq { ($got:expr, $expected:expr) => {
        assert_eq!(str::from_utf8(unsafe { CStr::from_ptr($got) }.to_bytes()).unwrap(), $expected);
    } }

    #[test]
    fn parse_args_should_abort_on_unknown_arguments() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = vec!(String::from("ja2"), String::from("testunknown"));
        assert_eq!(super::parse_args(&mut engine_options, &input).unwrap(), "Unknown arguments: 'testunknown'.");
    }

    #[test]
    fn parse_args_should_abort_on_unknown_switch() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = vec!(String::from("ja2"), String::from("--testunknown"));
        assert_eq!(super::parse_args(&mut engine_options, &input).unwrap(), "Unrecognized option: 'testunknown'");
    }

    #[test]
    fn parse_args_should_have_correct_fullscreen_default_value() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = vec!(String::from("ja2"));
        assert_eq!(super::parse_args(&mut engine_options, &input), None);
        assert!(!should_start_in_fullscreen(&engine_options));
    }

    #[test]
    fn parse_args_should_be_able_to_change_fullscreen_value() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = vec!(String::from("ja2"), String::from("-fullscreen"));
        assert_eq!(super::parse_args(&mut engine_options, &input), None);
        assert!(should_start_in_fullscreen(&engine_options));
    }

    #[test]
    fn parse_args_should_be_able_to_show_help() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = vec!(String::from("ja2"), String::from("-help"));
        assert_eq!(super::parse_args(&mut engine_options, &input), None);
        assert!(should_show_help(&engine_options));
    }

    #[test]
    fn parse_args_should_continue_with_multiple_known_switches() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = vec!(String::from("ja2"), String::from("-debug"), String::from("-mod"), String::from("a"), String::from("--mod"), String::from("ö"));
        assert_eq!(super::parse_args(&mut engine_options, &input), None);
        assert!(should_start_in_debug_mode(&engine_options));
        assert_eq!(get_number_of_mods(&engine_options), 2);
        assert_eq!(unsafe { CString::from_raw(get_mod(&engine_options, 0)) }, CString::new("a").unwrap());
        assert_eq!(unsafe { CString::from_raw(get_mod(&engine_options, 1)) }, CString::new("ö").unwrap());
    }

    #[test]
    fn parse_args_should_fail_with_unknown_resversion() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = vec!(String::from("ja2"), String::from("--resversion"), String::from("TESTUNKNOWN"));
        assert_eq!(super::parse_args(&mut engine_options, &input).unwrap(), "Resource version TESTUNKNOWN is unknown");
    }

    #[test]
    fn parse_args_should_return_the_correct_resversion_for_russian() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = vec!(String::from("ja2"), String::from("-resversion"), String::from("RUSSIAN"));
        assert_eq!(super::parse_args(&mut engine_options, &input), None);
        assert!(get_resource_version(&engine_options) == super::VanillaVersion::RUSSIAN);
    }

    #[test]
    fn parse_args_should_return_the_correct_resversion_for_italian() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = vec!(String::from("ja2"), String::from("-resversion"), String::from("ITALIAN"));
        assert_eq!(super::parse_args(&mut engine_options, &input), None);
        assert!(get_resource_version(&engine_options) == super::VanillaVersion::ITALIAN);
    }

    #[test]
    fn parse_args_should_return_the_correct_resolution() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = vec!(String::from("ja2"), String::from("--res"), String::from("1120x960"));
        assert_eq!(super::parse_args(&mut engine_options, &input), None);
        assert_eq!(get_resolution_x(&engine_options), 1120);
        assert_eq!(get_resolution_y(&engine_options), 960);
    }

    #[test]
    #[cfg(target_os = "macos")]
    fn parse_args_should_return_the_correct_canonical_game_dir_on_mac() {
        let mut engine_options: super::EngineOptions = Default::default();
        let temp_dir = tempdir::TempDir::new("ja2-tests").unwrap();
        let dir_path = temp_dir.path().join("foo");

        fs::create_dir_all(dir_path).unwrap();

        let input = vec!(String::from("ja2"), String::from("--gamedir"), String::from(temp_dir.path().join("foo/../foo/../").to_str().unwrap()));

        assert_eq!(super::parse_args(&mut engine_options, &input), None);
        let comp = str::from_utf8(unsafe { CStr::from_ptr(get_vanilla_game_dir(&engine_options)) }.to_bytes()).unwrap();
        let temp = fs::canonicalize(temp_dir.path()).expect("Problem during building of reference value.");
        let base = temp.to_str().unwrap();

        assert_eq!(comp, base);
    }

    #[test]
    #[cfg(all(not(windows), not(target_os = "macos")))]
    fn parse_args_should_return_the_correct_canonical_game_dir_on_linux() {
        let mut engine_options: super::EngineOptions = Default::default();
        let temp_dir = tempdir::TempDir::new("ja2-tests").unwrap();
        let dir_path = temp_dir.path().join("foo");

        fs::create_dir_all(dir_path).unwrap();

        let input = vec!(String::from("ja2"), String::from("--gamedir"), String::from(temp_dir.path().join("foo/../foo/../").to_str().unwrap()));

        assert_eq!(super::parse_args(&mut engine_options, &input), None);
        assert_eq!(str::from_utf8(unsafe { CStr::from_ptr(get_vanilla_game_dir(&engine_options)) }.to_bytes()).unwrap(), temp_dir.path().to_str().unwrap());
    }

    #[test]
    #[cfg(windows)]
    fn parse_args_should_return_the_correct_canonical_game_dir_on_windows() {
        let mut engine_options: super::EngineOptions = Default::default();
        let temp_dir = tempdir::TempDir::new("ja2-tests").unwrap();
        let dir_path = temp_dir.path().join("foo");

        fs::create_dir_all(dir_path).unwrap();

        let input = vec!(String::from("ja2"), String::from("--gamedir"), String::from(temp_dir.path().to_str().unwrap()));

        assert_eq!(super::parse_args(&mut engine_options, &input), None);
        assert_eq!(str::from_utf8(unsafe { CStr::from_ptr(get_vanilla_game_dir(&engine_options)) }.to_bytes()).unwrap(), temp_dir.path().to_str().unwrap());
    }

    #[test]
    fn parse_args_should_fail_with_non_existing_directory() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = vec!(String::from("ja2"), String::from("--gamedir"), String::from("somethingelse"));

        assert_eq!(super::parse_args(&mut engine_options, &input), Some(String::from("Please specify an existing gamedir.")));
    }

    fn write_temp_folder_with_ja2_json(contents: &[u8]) -> tempdir::TempDir {
        let dir = tempdir::TempDir::new("ja2-test").unwrap();
        let ja2_home_dir = dir.path().join(".ja2");
        let file_path = ja2_home_dir.join("ja2.json");

        fs::create_dir(ja2_home_dir).unwrap();
        let mut f = File::create(file_path).unwrap();
        f.write_all(contents).unwrap();
        f.sync_all().unwrap();

        return dir
    }

    #[test]
    fn ensure_json_config_existence_should_ensure_existence_of_config_dir() {
        let dir = tempdir::TempDir::new("ja2-tests").unwrap();
        let home_path = dir.path().join("ja2_home");
        let ja2json_path = home_path.join("ja2.json");

        super::ensure_json_config_existence(&home_path).unwrap();

        assert!(home_path.exists());
        assert!(ja2json_path.is_file());
    }

    #[test]
    fn ensure_json_config_existence_should_not_overwrite_existing_ja2json() {
        let dir = write_temp_folder_with_ja2_json(b"Test");
        let ja2json_path = dir.path().join(".ja2/ja2.json");

        super::ensure_json_config_existence(&PathBuf::from(dir.path())).unwrap();

        let mut f = File::open(ja2json_path.clone()).unwrap();
        let mut content: Vec<u8> = vec!();
        f.read_to_end(&mut content).unwrap();

        assert!(ja2json_path.is_file());
        assert_eq!(content, b"Test");
    }

    #[test]
    fn parse_json_config_should_fail_with_missing_file() {
        let temp_dir = tempdir::TempDir::new("ja2-tests").unwrap();
        let stracciatella_home = PathBuf::from(temp_dir.path());

        assert_eq!(super::parse_json_config(&stracciatella_home), Err(String::from("Error reading ja2.json config file: entity not found")));
    }

    #[test]
    fn parse_json_config_should_fail_with_invalid_json() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ not json }");
        let stracciatella_home = PathBuf::from(temp_dir.path().join(".ja2"));

        assert_eq!(super::parse_json_config(&stracciatella_home), Err(String::from("Error parsing ja2.json config file: key must be a string at line 1 column 3")));
    }

    #[test]
    fn parse_json_config_should_be_able_to_change_game_dir() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"game_dir\": \"/dd\" }");
        let engine_options = super::parse_json_config(&PathBuf::from(temp_dir.path().join(".ja2"))).unwrap();

        assert_chars_eq!(get_vanilla_game_dir(&engine_options), "/dd");
    }

    #[test]
    fn parse_json_config_should_be_able_to_change_game_dir_with_data_dir() {
        // data_dir is an alias to game_dir
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"data_dir\": \"/dd\" }");
        let engine_options = super::parse_json_config(&PathBuf::from(temp_dir.path().join(".ja2"))).unwrap();

        assert_chars_eq!(get_vanilla_game_dir(&engine_options), "/dd");
    }

    #[test]
    fn parse_json_config_should_be_able_to_change_fullscreen_value() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"fullscreen\": true }");
        let engine_options = super::parse_json_config(&PathBuf::from(temp_dir.path().join(".ja2"))).unwrap();

        assert!(should_start_in_fullscreen(&engine_options));
    }

    #[test]
    fn parse_json_config_should_be_able_to_change_debug_value() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"debug\": true }");
        let engine_options = super::parse_json_config(&PathBuf::from(temp_dir.path().join(".ja2"))).unwrap();

        assert!(should_start_in_debug_mode(&engine_options));
    }

    #[test]
    fn parse_json_config_should_be_able_to_start_without_sound() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"nosound\": true }");
        let engine_options = super::parse_json_config(&PathBuf::from(temp_dir.path().join(".ja2"))).unwrap();

        assert!(should_start_without_sound(&engine_options));
    }

    #[test]
    fn parse_json_config_should_not_be_able_to_run_help() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"help\": true, \"show_help\": true }");
        let engine_options = super::parse_json_config(&PathBuf::from(temp_dir.path().join(".ja2"))).unwrap();

        assert!(!should_show_help(&engine_options));
    }

    #[test]
    fn parse_json_config_should_not_be_able_to_run_unittests() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"unittests\": true, \"run_unittests\": true }");
        let engine_options = super::parse_json_config(&PathBuf::from(temp_dir.path().join(".ja2"))).unwrap();

        assert!(!should_run_unittests(&engine_options));
    }

    #[test]
    fn parse_json_config_should_not_be_able_to_run_editor() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"editor\": true, \"run_editor\": true }");
        let engine_options = super::parse_json_config(&PathBuf::from(temp_dir.path().join(".ja2"))).unwrap();

        assert!(!should_run_editor(&engine_options));
    }

    #[test]
    fn parse_json_config_should_fail_with_invalid_mod() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"mods\": [ \"a\", true ] }");
        let stracciatella_home = PathBuf::from(temp_dir.path().join(".ja2"));

        assert_eq!(super::parse_json_config(&stracciatella_home), Err(String::from("Error parsing ja2.json config file: invalid type: boolean `true`, expected a string at line 1 column 21")));
    }

    #[test]
    fn parse_json_config_should_continue_with_multiple_known_switches() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"debug\": true, \"mods\": [ \"m1\", \"a2\" ] }");
        let engine_options = super::parse_json_config(&PathBuf::from(temp_dir.path().join(".ja2"))).unwrap();

        assert!(should_start_in_debug_mode(&engine_options));
        assert!(get_number_of_mods(&engine_options) == 2);
    }

    #[test]
    fn parse_json_config_should_fail_with_unknown_resversion() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"resversion\": \"TESTUNKNOWN\" }");
        let stracciatella_home = PathBuf::from(temp_dir.path().join(".ja2"));

        assert_eq!(super::parse_json_config(&stracciatella_home), Err(String::from("Error parsing ja2.json config file: unknown variant `TESTUNKNOWN`, expected one of `DUTCH`, `ENGLISH`, `FRENCH`, `GERMAN`, `ITALIAN`, `POLISH`, `RUSSIAN`, `RUSSIAN_GOLD` at line 1 column 29")));
    }

    #[test]
    fn parse_json_config_should_return_the_correct_resversion_for_russian() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"resversion\": \"RUSSIAN\" }");
        let engine_options = super::parse_json_config(&PathBuf::from(temp_dir.path().join(".ja2"))).unwrap();

        assert_eq!(get_resource_version(&engine_options), super::VanillaVersion::RUSSIAN);
    }

    #[test]
    fn parse_json_config_should_return_the_correct_resversion_for_italian() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"resversion\": \"ITALIAN\" }");
        let engine_options = super::parse_json_config(&PathBuf::from(temp_dir.path().join(".ja2"))).unwrap();

        assert_eq!(get_resource_version(&engine_options), super::VanillaVersion::ITALIAN);
    }

    #[test]
    fn parse_json_config_should_return_the_correct_resolution() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"res\": \"1024x768\" }");
        let engine_options = super::parse_json_config(&PathBuf::from(temp_dir.path().join(".ja2"))).unwrap();

        assert_eq!(get_resolution_x(&engine_options), 1024);
        assert_eq!(get_resolution_y(&engine_options), 768);
    }

    #[test]
    #[cfg(not(windows))]
    fn find_stracciatella_home_should_find_the_correct_stracciatella_home_path_on_unixlike() {
        let mut engine_options: super::EngineOptions = Default::default();
        engine_options.stracciatella_home = super::find_stracciatella_home().unwrap();

        assert_eq!(str::from_utf8(unsafe { CStr::from_ptr(get_stracciatella_home(&engine_options)) }.to_bytes()).unwrap(), format!("{}/.ja2", env::var("HOME").unwrap()));
    }

    #[test]
    #[cfg(windows)]
    fn find_stracciatella_home_should_find_the_correct_stracciatella_home_path_on_windows() {
        use self::regex::Regex;

        let mut engine_options: super::EngineOptions = Default::default();
        engine_options.stracciatella_home = super::find_stracciatella_home().unwrap();

        let result = str::from_utf8(unsafe { CStr::from_ptr(get_stracciatella_home(&engine_options)) }.to_bytes()).unwrap();
        let regex = Regex::new(r"^[A-Z]:\\(.*)+\\JA2").unwrap();
        assert!(regex.is_match(result), "{} is not a valid home dir for windows", result);
    }

    #[test]
    fn build_engine_options_from_home_and_args_should_overwrite_json_with_command_line_args() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"game_dir\": \"/some/place/where/the/data/is\", \"res\": \"1024x768\", \"fullscreen\": true }");
        let args = vec!(String::from("ja2"), String::from("--res"), String::from("1100x480"));
        let home = temp_dir.path().join(".ja2");

        let engine_options = super::EngineOptions::from_home_and_args(&home, &args).unwrap();

        assert_eq!(get_resolution_x(&engine_options), 1100);
        assert_eq!(get_resolution_y(&engine_options), 480);
        assert_eq!(should_start_in_fullscreen(&engine_options), true);
    }

    #[test]
    fn build_engine_options_from_home_and_args_should_return_an_error_if_datadir_is_not_set() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"res\": \"1024x768\", \"fullscreen\": true }");
        let args = vec!(String::from("ja2"), String::from("--res"), String::from("1100x480"));
        let home = temp_dir.path().join(".ja2");
        let expected_error_message = "Vanilla data directory has to be set either in config file or per command line switch";

        let engine_options_res = super::EngineOptions::from_home_and_args(&home, &args);

        assert_eq!(engine_options_res, Err(String::from(expected_error_message)));
    }

    #[test]
    fn write_engine_options_should_write_a_json_file_that_can_be_serialized_again() {
        let mut engine_options = super::EngineOptions::default();
        let temp_dir = write_temp_folder_with_ja2_json(b"Invalid JSON");
        let stracciatella_home = PathBuf::from(temp_dir.path().join(".ja2"));

        engine_options.stracciatella_home = stracciatella_home.clone();
        engine_options.resolution = super::Resolution(100, 100);

        write_engine_options(&mut engine_options);

        let got_engine_options = super::parse_json_config(&stracciatella_home).unwrap();

        assert_eq!(got_engine_options.resolution, engine_options.resolution);
    }

    #[test]
    fn write_engine_options_should_write_a_pretty_json_file() {
        let mut engine_options = super::EngineOptions::default();
        let temp_dir = write_temp_folder_with_ja2_json(b"Invalid JSON");
        let stracciatella_home = PathBuf::from(temp_dir.path().join(".ja2"));
        let stracciatella_json = PathBuf::from(temp_dir.path().join(".ja2/ja2.json"));

        engine_options.stracciatella_home = stracciatella_home.clone();
        engine_options.resolution = super::Resolution(100, 100);

        write_engine_options(&mut engine_options);

        let mut config_file_contents = String::from("");
        File::open(stracciatella_json).unwrap().read_to_string(&mut config_file_contents).unwrap();

        assert_eq!(config_file_contents,
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
}"##);
    }

    #[test]
    fn get_resource_version_string_should_return_the_correct_resource_version_string() {
        assert_chars_eq!(get_resource_version_string(super::VanillaVersion::DUTCH), "Dutch");
        assert_chars_eq!(get_resource_version_string(super::VanillaVersion::ENGLISH), "English");
        assert_chars_eq!(get_resource_version_string(super::VanillaVersion::FRENCH), "French");
        assert_chars_eq!(get_resource_version_string(super::VanillaVersion::GERMAN), "German");
        assert_chars_eq!(get_resource_version_string(super::VanillaVersion::ITALIAN), "Italian");
        assert_chars_eq!(get_resource_version_string(super::VanillaVersion::POLISH), "Polish");
        assert_chars_eq!(get_resource_version_string(super::VanillaVersion::RUSSIAN), "Russian");
        assert_chars_eq!(get_resource_version_string(super::VanillaVersion::RUSSIAN_GOLD), "Russian (Gold)");

    }
}
