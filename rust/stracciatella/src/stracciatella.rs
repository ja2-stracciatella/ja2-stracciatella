//! This crate contains the rust code of ja2-stracciatella.
//!
//! The crate [stracciatella_c_api] contains the C API of this crate.
//!
//! [stracciatella_c_api]: ../stracciatella_c_api/index.html

pub mod config;
pub mod file_formats;
pub mod fs;
pub mod guess;
pub mod json;
pub mod librarydb;
pub mod logger;
pub mod res;
pub mod unicode;

use std::default::Default;
use std::path::PathBuf;

use log::warn;

use crate::config::{Cli, EngineOptions, Ja2Json};
use crate::fs::canonicalize;

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
pub fn get_assets_dir() -> PathBuf {
    if let Some(extra_data_dir) = option_env!("EXTRA_DATA_DIR") {
        if !extra_data_dir.is_empty() {
            // use directory defined at compile time
            return extra_data_dir.into();
        }
    }
    match std::env::current_exe().and_then(canonicalize) {
        Ok(exe) => {
            // use directory of the executable
            if let Some(dir) = exe.parent() {
                dir.into()
            } else {
                ".".into()
            }
        }
        Err(err) => {
            warn!(
                "Defaulting assets dir to the current directory. Reason: {:?}",
                err
            );
            ".".into()
        }
    }
}

#[cfg(test)]
mod tests {
    use std::fs;
    use std::fs::File;
    use std::io::prelude::*;
    use std::path::Path;

    use tempfile::TempDir;

    use crate::config::{find_stracciatella_home, VanillaVersion};
    use crate::*;

    #[test]
    fn parse_args_should_abort_on_unknown_arguments() {
        let mut engine_options = EngineOptions::default();
        let input = vec![String::from("ja2"), String::from("testunknown")];
        assert_eq!(
            parse_args(&mut engine_options, &input).unwrap(),
            "Unknown arguments: 'testunknown'."
        );
    }

    #[test]
    fn parse_args_should_abort_on_unknown_switch() {
        let mut engine_options = EngineOptions::default();
        let input = vec![String::from("ja2"), String::from("--testunknown")];
        assert_eq!(
            parse_args(&mut engine_options, &input).unwrap(),
            format!(
                "{}\n{}",
                "Unrecognized option: 'testunknown'",
                &Cli::usage()
            )
        );
    }

    #[test]
    fn parse_args_should_have_correct_fullscreen_default_value() {
        let mut engine_options = EngineOptions::default();
        let input = vec![String::from("ja2")];
        assert_eq!(parse_args(&mut engine_options, &input), None);
        assert_eq!(engine_options.start_in_fullscreen, false);
    }

    #[test]
    fn parse_args_should_be_able_to_change_fullscreen_value() {
        let mut engine_options = EngineOptions::default();
        let input = vec![String::from("ja2"), String::from("-fullscreen")];
        assert_eq!(parse_args(&mut engine_options, &input), None);
        assert_eq!(engine_options.start_in_fullscreen, true);
    }

    #[test]
    fn parse_args_should_be_able_to_show_help() {
        let mut engine_options = EngineOptions::default();
        let input = vec![String::from("ja2"), String::from("-help")];
        assert_eq!(parse_args(&mut engine_options, &input), None);
        assert_eq!(engine_options.show_help, true);
    }

    #[test]
    fn parse_args_should_continue_with_multiple_known_switches() {
        let mut engine_options = EngineOptions::default();
        let input = vec![
            String::from("ja2"),
            String::from("-debug"),
            String::from("-mod"),
            String::from("a"),
            String::from("--mod"),
            String::from("ö"),
        ];
        assert_eq!(parse_args(&mut engine_options, &input), None);
        assert_eq!(engine_options.start_in_debug_mode, true);
        assert_eq!(engine_options.mods.len(), 2);
        assert_eq!(engine_options.mods[0], "a");
        assert_eq!(engine_options.mods[1], "ö");
    }

    #[test]
    fn parse_args_should_fail_with_unknown_resversion() {
        let mut engine_options = EngineOptions::default();
        let input = vec![
            String::from("ja2"),
            String::from("--resversion"),
            String::from("TESTUNKNOWN"),
        ];
        assert_eq!(
            parse_args(&mut engine_options, &input).unwrap(),
            "Resource version TESTUNKNOWN is unknown"
        );
    }

    #[test]
    fn parse_args_should_return_the_correct_resversion_for_russian() {
        let mut engine_options = EngineOptions::default();
        let input = vec![
            String::from("ja2"),
            String::from("-resversion"),
            String::from("RUSSIAN"),
        ];
        assert_eq!(parse_args(&mut engine_options, &input), None);
        assert_eq!(engine_options.resource_version, VanillaVersion::RUSSIAN);
    }

    #[test]
    fn parse_args_should_return_the_correct_resversion_for_italian() {
        let mut engine_options = EngineOptions::default();
        let input = vec![
            String::from("ja2"),
            String::from("-resversion"),
            String::from("ITALIAN"),
        ];
        assert_eq!(parse_args(&mut engine_options, &input), None);
        assert_eq!(engine_options.resource_version, VanillaVersion::ITALIAN);
    }

    #[test]
    fn parse_args_should_return_the_correct_resolution() {
        let mut engine_options = EngineOptions::default();
        let input = vec![
            String::from("ja2"),
            String::from("--res"),
            String::from("1120x960"),
        ];
        assert_eq!(parse_args(&mut engine_options, &input), None);
        assert_eq!(engine_options.resolution.0, 1120);
        assert_eq!(engine_options.resolution.1, 960);
    }

    #[test]
    #[cfg(target_os = "macos")]
    fn parse_args_should_return_the_correct_canonical_game_dir_on_mac() {
        let mut engine_options = EngineOptions::default();
        let temp_dir = TempDir::new().unwrap();
        let dir_path = temp_dir.path().join("foo");

        fs::create_dir_all(dir_path).unwrap();

        let input = vec![
            String::from("ja2"),
            String::from("--gamedir"),
            String::from(temp_dir.path().join("foo/../foo/../").to_str().unwrap()),
        ];

        assert_eq!(parse_args(&mut engine_options, &input), None);
        let comp = engine_options.vanilla_game_dir;
        let base =
            canonicalize(temp_dir.path()).expect("Problem during building of reference value.");

        assert_eq!(comp, base);
    }

    #[test]
    #[cfg(all(not(windows), not(target_os = "macos")))]
    fn parse_args_should_return_the_correct_canonical_game_dir_on_linux() {
        let mut engine_options = EngineOptions::default();
        let temp_dir = TempDir::new().unwrap();
        let dir_path = temp_dir.path().join("foo");

        fs::create_dir_all(dir_path).unwrap();

        let input = vec![
            String::from("ja2"),
            String::from("--gamedir"),
            String::from(temp_dir.path().join("foo/../foo/../").to_str().unwrap()),
        ];

        assert_eq!(parse_args(&mut engine_options, &input), None);
        assert_eq!(engine_options.vanilla_game_dir, temp_dir.path());
    }

    #[test]
    #[cfg(windows)]
    fn parse_args_should_return_the_correct_canonical_game_dir_on_windows() {
        let mut engine_options = EngineOptions::default();
        let temp_dir = TempDir::new().unwrap();
        let dir_path = temp_dir.path().join("foo");

        fs::create_dir_all(dir_path).unwrap();

        let input = vec![
            String::from("ja2"),
            String::from("--gamedir"),
            String::from(temp_dir.path().to_str().unwrap()),
        ];

        assert_eq!(parse_args(&mut engine_options, &input), None);
        assert_eq!(engine_options.vanilla_game_dir, temp_dir.path());
    }

    #[test]
    fn parse_args_should_fail_with_non_existing_directory() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = vec![
            String::from("ja2"),
            String::from("--gamedir"),
            String::from("somethingelse"),
        ];

        assert_eq!(
            parse_args(&mut engine_options, &input),
            Some(String::from("Please specify an existing gamedir."))
        );
    }

    pub fn write_temp_folder_with_ja2_json(contents: &[u8]) -> TempDir {
        let dir = TempDir::new().unwrap();
        let ja2_home_dir = dir.path().join(".ja2");
        let file_path = ja2_home_dir.join("ja2.json");

        fs::create_dir(ja2_home_dir).unwrap();
        let mut f = File::create(file_path).unwrap();
        f.write_all(contents).unwrap();
        f.sync_all().unwrap();

        dir
    }

    #[test]
    fn ensure_json_config_existence_should_ensure_existence_of_config_dir() {
        let dir = TempDir::new().unwrap();
        let home_path = dir.path().join("ja2_home");
        let ja2json_path = home_path.join("ja2.json");

        ensure_json_config_existence(&home_path).unwrap();

        assert!(home_path.exists());
        assert!(ja2json_path.is_file());
    }

    #[test]
    fn ensure_json_config_existence_should_not_overwrite_existing_ja2json() {
        let dir = write_temp_folder_with_ja2_json(b"Test");
        let ja2json_path = dir.path().join(".ja2/ja2.json");

        ensure_json_config_existence(&dir.path().to_owned()).unwrap();

        let mut f = File::open(ja2json_path.clone()).unwrap();
        let mut content: Vec<u8> = vec![];
        f.read_to_end(&mut content).unwrap();

        assert!(ja2json_path.is_file());
        assert_eq!(content, b"Test");
    }

    #[test]
    fn parse_json_config_should_fail_with_missing_file() {
        let temp_dir = TempDir::new().unwrap();
        let stracciatella_home = temp_dir.path().to_owned();
        let expected = Err("Error reading ja2.json config file: entity not found".to_owned());

        assert_eq!(parse_json_config(&stracciatella_home), expected);
    }

    #[test]
    fn parse_json_config_should_fail_with_invalid_json() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ not json }");
        let stracciatella_home = temp_dir.path().join(".ja2");

        assert_eq!(
            parse_json_config(&stracciatella_home),
            Err(String::from(
                "Error parsing ja2.json config file: key must be a string at line 1 column 3"
            ))
        );
    }

    #[test]
    fn parse_json_config_should_be_able_to_change_game_dir() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"game_dir\": \"/dd\" }");
        let engine_options = parse_json_config(&temp_dir.path().join(".ja2")).unwrap();

        assert_eq!(engine_options.vanilla_game_dir, Path::new("/dd"));
    }

    #[test]
    fn parse_json_config_should_be_able_to_change_game_dir_with_data_dir() {
        // data_dir is an alias to game_dir
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"data_dir\": \"/dd\" }");
        let engine_options = parse_json_config(&temp_dir.path().join(".ja2")).unwrap();

        assert_eq!(engine_options.vanilla_game_dir, Path::new("/dd"));
    }

    #[test]
    fn parse_json_config_should_be_able_to_change_fullscreen_value() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"fullscreen\": true }");
        let engine_options = parse_json_config(&temp_dir.path().join(".ja2")).unwrap();

        assert_eq!(engine_options.start_in_fullscreen, true);
    }

    #[test]
    fn parse_json_config_should_be_able_to_change_debug_value() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"debug\": true }");
        let engine_options = parse_json_config(&temp_dir.path().join(".ja2")).unwrap();

        assert_eq!(engine_options.start_in_debug_mode, true);
    }

    #[test]
    fn parse_json_config_should_be_able_to_start_without_sound() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"nosound\": true }");
        let engine_options = parse_json_config(&temp_dir.path().join(".ja2")).unwrap();

        assert_eq!(engine_options.start_without_sound, true);
    }

    #[test]
    fn parse_json_config_should_not_be_able_to_run_help() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"help\": true, \"show_help\": true }");
        let engine_options = parse_json_config(&temp_dir.path().join(".ja2")).unwrap();

        assert_eq!(engine_options.show_help, false);
    }

    #[test]
    fn parse_json_config_should_not_be_able_to_run_unittests() {
        let temp_dir =
            write_temp_folder_with_ja2_json(b"{ \"unittests\": true, \"run_unittests\": true }");
        let engine_options = parse_json_config(&temp_dir.path().join(".ja2")).unwrap();

        assert_eq!(engine_options.run_unittests, false);
    }

    #[test]
    fn parse_json_config_should_not_be_able_to_run_editor() {
        let temp_dir =
            write_temp_folder_with_ja2_json(b"{ \"editor\": true, \"run_editor\": true }");
        let engine_options = parse_json_config(&temp_dir.path().join(".ja2")).unwrap();

        assert_eq!(engine_options.run_editor, false);
    }

    #[test]
    fn parse_json_config_should_fail_with_invalid_mod() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"mods\": [ \"a\", true ] }");
        let stracciatella_home = temp_dir.path().join(".ja2");

        assert_eq!(parse_json_config(&stracciatella_home), Err(String::from("Error parsing ja2.json config file: invalid type: boolean `true`, expected a string at line 1 column 21")));
    }

    #[test]
    fn parse_json_config_should_continue_with_multiple_known_switches() {
        let temp_dir =
            write_temp_folder_with_ja2_json(b"{ \"debug\": true, \"mods\": [ \"m1\", \"a2\" ] }");
        let engine_options = parse_json_config(&temp_dir.path().join(".ja2")).unwrap();

        assert_eq!(engine_options.start_in_debug_mode, true);
        assert_eq!(engine_options.mods.len(), 2);
    }

    #[test]
    fn parse_json_config_should_fail_with_unknown_resversion() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"resversion\": \"TESTUNKNOWN\" }");
        let stracciatella_home = temp_dir.path().join(".ja2");

        assert_eq!(parse_json_config(&stracciatella_home), Err(String::from("Error parsing ja2.json config file: unknown variant `TESTUNKNOWN`, expected one of `DUTCH`, `ENGLISH`, `FRENCH`, `GERMAN`, `ITALIAN`, `POLISH`, `RUSSIAN`, `RUSSIAN_GOLD` at line 1 column 29")));
    }

    #[test]
    fn parse_json_config_should_return_the_correct_resversion_for_russian() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"resversion\": \"RUSSIAN\" }");
        let engine_options = parse_json_config(&temp_dir.path().join(".ja2")).unwrap();

        assert_eq!(engine_options.resource_version, VanillaVersion::RUSSIAN);
    }

    #[test]
    fn parse_json_config_should_return_the_correct_resversion_for_italian() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"resversion\": \"ITALIAN\" }");
        let engine_options = parse_json_config(&temp_dir.path().join(".ja2")).unwrap();

        assert_eq!(engine_options.resource_version, VanillaVersion::ITALIAN);
    }

    #[test]
    fn parse_json_config_should_return_the_correct_resolution() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"res\": \"1024x768\" }");
        let engine_options = parse_json_config(&temp_dir.path().join(".ja2")).unwrap();

        assert_eq!(engine_options.resolution.0, 1024);
        assert_eq!(engine_options.resolution.1, 768);
    }

    #[test]
    #[cfg(not(windows))]
    fn find_stracciatella_home_should_find_the_correct_stracciatella_home_path_on_unixlike() {
        let mut engine_options = EngineOptions::default();
        engine_options.stracciatella_home = find_stracciatella_home().unwrap();
        let expected = format!("{}/.ja2", std::env::var("HOME").unwrap());

        assert_eq!(engine_options.stracciatella_home, Path::new(&expected));
    }

    #[test]
    #[cfg(windows)]
    fn find_stracciatella_home_should_find_the_correct_stracciatella_home_path_on_windows() {
        use regex::Regex;

        let mut engine_options = EngineOptions::default();
        engine_options.stracciatella_home = find_stracciatella_home().unwrap();

        let result = engine_options.stracciatella_home.to_str().unwrap();
        let regex = Regex::new(r"^[A-Z]:\\(.*)+\\JA2").unwrap();
        assert!(
            regex.is_match(result),
            "{} is not a valid home dir for windows",
            result
        );
    }

    #[test]
    fn build_engine_options_from_home_and_args_should_overwrite_json_with_command_line_args() {
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"game_dir\": \"/some/place/where/the/data/is\", \"res\": \"1024x768\", \"fullscreen\": true }");
        let args = vec![
            String::from("ja2"),
            String::from("--res"),
            String::from("1100x480"),
        ];
        let home = temp_dir.path().join(".ja2");

        let engine_options = EngineOptions::from_home_and_args(&home, &args).unwrap();

        assert_eq!(engine_options.resolution.0, 1100);
        assert_eq!(engine_options.resolution.1, 480);
        assert_eq!(engine_options.start_in_fullscreen, true);
    }

    #[test]
    fn build_engine_options_from_home_and_args_should_return_an_error_if_datadir_is_not_set() {
        let temp_dir =
            write_temp_folder_with_ja2_json(b"{ \"res\": \"1024x768\", \"fullscreen\": true }");
        let args = vec![
            String::from("ja2"),
            String::from("--res"),
            String::from("1100x480"),
        ];
        let home = temp_dir.path().join(".ja2");
        let expected_error_message =
            "Vanilla data directory has to be set either in config file or per command line switch";

        let engine_options_res = EngineOptions::from_home_and_args(&home, &args);

        assert_eq!(
            engine_options_res,
            Err(String::from(expected_error_message))
        );
    }
}
