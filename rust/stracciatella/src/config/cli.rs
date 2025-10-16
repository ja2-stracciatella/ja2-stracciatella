//! This module contains code to read command line arguments for the ja2 executable

use std::path::PathBuf;
use std::str::FromStr;

use getopts::Options;
use log::warn;

use crate::config::{EngineOptions, Resolution, VanillaVersion};
use crate::fs::canonicalize;

#[cfg(not(windows))]
static GAME_DIR_OPTION_EXAMPLE: &str = "/opt/ja2";

#[cfg(windows)]
static GAME_DIR_OPTION_EXAMPLE: &str = "C:\\JA2";

#[derive(Debug, Clone, PartialEq, Eq)]
/// An error that can occur when parsing and evaluating CLI arguments
pub enum CliError {
    // Unknown arguments were passed to the CLI
    UnknownArguments(Vec<String>),
    // Missing game dir
    GameDirDoesNotExist(String),
    // Invalid arguments were passed to the CLI
    InvalidValue(String, String),
    // Parsing CLI arguments failed
    ParsingFailed(String),
}

impl std::fmt::Display for CliError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::UnknownArguments(args) => write!(f, "Unknown arguments: `{}`.", args.join(" ")),
            Self::GameDirDoesNotExist(dir) => write!(f, "The gamedir `{}` does not exist.", dir),
            Self::InvalidValue(name, message) => {
                write!(f, "Invalid value for argument `{}`: {}.", name, message)
            }
            Self::ParsingFailed(e) => write!(f, "Parsing CLI arguments failed: {}", e),
        }
    }
}

impl std::error::Error for CliError {}

/// Handles command line parameters for executables
///
/// Encapsulates the Cli arguments definition and the actual CLI arguments
/// passed to the executable
pub struct Cli {
    args: Vec<String>,
    options: Options,
}

impl Cli {
    /// Constructor to create Cli instance from command line arguments
    pub fn from_args(args: &[String]) -> Self {
        let mut opts = Options::new();

        opts.long_only(true);
        opts.optmulti(
            "",
            "datadir", // TODO remove this deprecated option in version >= 0.18
            "Set path for the vanilla game directory. DEPRECATED use -gamedir instead.",
            GAME_DIR_OPTION_EXAMPLE,
        );
        opts.optmulti(
            "",
            "gamedir",
            "Set path for the vanilla game directory",
            GAME_DIR_OPTION_EXAMPLE,
        );
        opts.optmulti(
            "",
            "mod",
            "Start one of the game modifications. MOD_NAME is the name of modification, e.g. 'from-russia-with-love'. See mods folder for possible options.",
            "MOD_NAME"
        );
        opts.optopt(
            "",
            "res",
            "Screen resolution, e.g. 800x600. Default value is 640x480",
            "WIDTHxHEIGHT",
        );
        opts.optopt(
            "",
            "brightness",
            "Screen brightness (gamma multiplier) value to set where 0.0 is completely dark and 1.0 is normal brightness. Set to a negative value to disable brightness adjustment. Default value is -1",
            "GAMMA_VALUE"
        );
        opts.optopt(
            "",
            "resversion",
            "Version of the game resources. Possible values: DUTCH, ENGLISH, FRENCH, GERMAN, ITALIAN, POLISH, RUSSIAN, RUSSIAN_GOLD, SIMPLIFIED_CHINESE. Default value is ENGLISH. RUSSIAN is for BUKA Agonia Vlasty release. RUSSIAN_GOLD is for Gold release",
            "RUSSIAN_GOLD"
        );
        opts.optflag(
            "",
            "unittests",
            "Perform unit tests. E.g. 'ja2.exe -unittests --gtest_output=\"xml:report.xml\" --gtest_repeat=2'");
        opts.optflag(
            "",
            "editor",
            "Start the map editor (Editor.slf is required)",
        );
        opts.optflag("", "fullscreen", "Start the game in the fullscreen mode");
        opts.optflag("", "nosound", "Turn the sound and music off");
        opts.optflag("", "window", "Start the game in a window");
        opts.optflag("", "debug", "Enable Debug Mode");
        opts.optflag("", "enumgen", "Generate enums for Lua and exit");
        opts.optflag("h", "help", "print this help menu");

        Cli {
            args: args.to_vec(),
            options: opts,
        }
    }

    /// Apply current arguments to EngineOptions struct
    pub fn apply_to_engine_options(
        &self,
        engine_options: &mut EngineOptions,
    ) -> Result<(), CliError> {
        match self.options.parse(&self.args[1..]) {
            Ok(m) => {
                if !m.free.is_empty() {
                    return Err(CliError::UnknownArguments(m.free));
                }

                if m.opt_str("datadir").is_some() {
                    warn!(
                        "The `datadir` command line argument is deprecated, use `gamedir` instead"
                    );
                }

                if let Some(s) = m.opts_str(&["gamedir".to_owned(), "datadir".to_owned()]) {
                    match canonicalize(PathBuf::from(s.as_str())) {
                        Ok(s) => {
                            let mut temp = String::from(s.to_str().expect("Should not happen"));
                            // remove UNC path prefix (Windows)
                            if temp.starts_with("\\\\") {
                                temp.drain(..2);
                                let pos = temp.find('\\').unwrap() + 1;
                                temp.drain(..pos);
                            }
                            engine_options.vanilla_game_dir = PathBuf::from(temp)
                        }
                        Err(_) => return Err(CliError::GameDirDoesNotExist(s)),
                    };
                }

                if !m.opt_strs("mod").is_empty() {
                    engine_options.mods =
                        m.opt_strs("mod").iter().map(|v| v.to_lowercase()).collect();
                }

                if let Some(s) = m.opt_str("res") {
                    match Resolution::from_str(&s) {
                        Ok(res) => {
                            engine_options.resolution = res;
                        }
                        Err(s) => return Err(CliError::InvalidValue("res".to_string(), s)),
                    }
                }

                if let Some(s) = m.opt_str("brightness") {
                    match s.parse::<f32>() {
                        Ok(val) => {
                            engine_options.brightness = val;
                        }
                        Err(_e) => {
                            return Err(CliError::InvalidValue(
                                "brighness".to_owned(),
                                "Should be a floating point value.".to_owned(),
                            ));
                        }
                    }
                }

                if let Some(s) = m.opt_str("resversion") {
                    match VanillaVersion::from_str(&s) {
                        Ok(resource_version) => engine_options.resource_version = resource_version,
                        Err(s) => return Err(CliError::InvalidValue("resversion".to_owned(), s)),
                    }
                }

                if m.opt_present("help") {
                    engine_options.show_help = true;
                }

                if m.opt_present("unittests") {
                    engine_options.run_unittests = true;
                }

                if m.opt_present("editor") {
                    engine_options.run_editor = true;
                }

                if m.opt_present("fullscreen") {
                    engine_options.start_in_fullscreen = true;
                    engine_options.start_in_window = false;
                }

                if m.opt_present("nosound") {
                    engine_options.start_without_sound = true;
                }

                if m.opt_present("window") {
                    engine_options.start_in_fullscreen = false;
                    engine_options.start_in_window = true;
                }

                if m.opt_present("debug") {
                    engine_options.start_in_debug_mode = true;
                }

                if m.opt_present("enumgen") {
                    engine_options.run_enum_gen = true;
                }

                Ok(())
            }
            Err(f) => Err(CliError::ParsingFailed(f.to_string())),
        }
    }

    /// Get command line usage string
    pub fn usage() -> String {
        let cli = Cli::from_args(&[]);
        cli.options.usage("Usage: ja2 [options]")
    }
}

#[cfg(test)]
mod tests {
    use tempfile::TempDir;

    use super::*;
    use crate::config::VanillaVersion;
    use crate::fs;

    #[test]
    fn apply_to_engine_options_should_abort_on_unknown_arguments() {
        let mut engine_options = EngineOptions::default();
        let input = Cli::from_args(&[String::from("ja2"), String::from("testunknown")]);
        assert_eq!(
            input
                .apply_to_engine_options(&mut engine_options)
                .err()
                .unwrap(),
            CliError::UnknownArguments(vec!["testunknown".to_owned()])
        );
    }

    #[test]
    fn apply_to_engine_options_should_abort_on_unknown_switch() {
        let mut engine_options = EngineOptions::default();
        let input = Cli::from_args(&[String::from("ja2"), String::from("--testunknown")]);
        assert_eq!(
            input
                .apply_to_engine_options(&mut engine_options)
                .err()
                .unwrap(),
            CliError::ParsingFailed("Unrecognized option: 'testunknown'".to_owned())
        );
    }

    #[test]
    fn apply_to_engine_options_should_have_correct_fullscreen_default_value() {
        let mut engine_options = EngineOptions::default();
        let input = Cli::from_args(&[String::from("ja2")]);
        assert_eq!(
            input.apply_to_engine_options(&mut engine_options).err(),
            None
        );
        assert!(!engine_options.start_in_fullscreen);
    }

    #[test]
    fn apply_to_engine_options_should_be_able_to_change_fullscreen_value() {
        let mut engine_options = EngineOptions::default();
        let input = Cli::from_args(&[String::from("ja2"), String::from("-fullscreen")]);
        assert_eq!(
            input.apply_to_engine_options(&mut engine_options).err(),
            None
        );
        assert!(engine_options.start_in_fullscreen);
    }

    #[test]
    fn apply_to_engine_options_should_be_able_to_show_help() {
        let mut engine_options = EngineOptions::default();
        let input = Cli::from_args(&[String::from("ja2"), String::from("-help")]);
        assert_eq!(
            input.apply_to_engine_options(&mut engine_options).err(),
            None
        );
        assert!(engine_options.show_help);
    }

    #[test]
    fn apply_to_engine_options_should_continue_with_multiple_known_switches() {
        let mut engine_options = EngineOptions::default();
        let input = Cli::from_args(&[
            String::from("ja2"),
            String::from("-debug"),
            String::from("-mod"),
            String::from("a"),
            String::from("--mod"),
            String::from("ö"),
        ]);
        assert_eq!(
            input.apply_to_engine_options(&mut engine_options).err(),
            None
        );
        assert!(engine_options.start_in_debug_mode);
        assert_eq!(engine_options.mods.len(), 2);
        assert_eq!(engine_options.mods[0], "a");
        assert_eq!(engine_options.mods[1], "ö");
    }

    #[test]
    fn apply_to_engine_options_should_fail_with_unknown_resversion() {
        let mut engine_options = EngineOptions::default();
        let input = Cli::from_args(&[
            String::from("ja2"),
            String::from("--resversion"),
            String::from("TESTUNKNOWN"),
        ]);
        assert_eq!(
            input
                .apply_to_engine_options(&mut engine_options)
                .err()
                .unwrap(),
            CliError::InvalidValue(
                "resversion".to_owned(),
                "Resource version TESTUNKNOWN is unknown".to_owned()
            )
        );
    }

    #[test]
    fn apply_to_engine_options_should_return_the_correct_resversion_for_russian() {
        let mut engine_options = EngineOptions::default();
        let input = Cli::from_args(&[
            String::from("ja2"),
            String::from("-resversion"),
            String::from("RUSSIAN"),
        ]);
        assert_eq!(
            input.apply_to_engine_options(&mut engine_options).err(),
            None
        );
        assert_eq!(engine_options.resource_version, VanillaVersion::RUSSIAN);
    }

    #[test]
    fn apply_to_engine_options_should_return_the_correct_resversion_for_italian() {
        let mut engine_options = EngineOptions::default();
        let input = Cli::from_args(&[
            String::from("ja2"),
            String::from("-resversion"),
            String::from("ITALIAN"),
        ]);
        assert_eq!(
            input.apply_to_engine_options(&mut engine_options).err(),
            None
        );
        assert_eq!(engine_options.resource_version, VanillaVersion::ITALIAN);
    }

    #[test]
    fn apply_to_engine_options_should_return_the_correct_resolution() {
        let mut engine_options = EngineOptions::default();
        let input = Cli::from_args(&[
            String::from("ja2"),
            String::from("--res"),
            String::from("1120x960"),
        ]);
        assert_eq!(
            input.apply_to_engine_options(&mut engine_options).err(),
            None
        );
        assert_eq!(engine_options.resolution.0, 1120);
        assert_eq!(engine_options.resolution.1, 960);
    }

    #[test]
    #[cfg(target_os = "macos")]
    fn apply_to_engine_options_should_return_the_correct_canonical_game_dir_on_mac() {
        let mut engine_options = EngineOptions::default();
        let temp_dir = TempDir::new().unwrap();
        let dir_path = temp_dir.path().join("foo");

        fs::create_dir_all(dir_path).unwrap();

        let input = Cli::from_args(&[
            String::from("ja2"),
            String::from("--gamedir"),
            String::from(temp_dir.path().join("foo/../foo/../").to_str().unwrap()),
        ]);

        assert_eq!(
            input.apply_to_engine_options(&mut engine_options).err(),
            None
        );
        let comp = engine_options.vanilla_game_dir;
        let base =
            canonicalize(temp_dir.path()).expect("Problem during building of reference value.");

        assert_eq!(comp, base);
    }

    #[test]
    #[cfg(all(not(windows), not(target_os = "macos")))]
    fn apply_to_engine_options_should_return_the_correct_canonical_game_dir_on_linux() {
        let mut engine_options = EngineOptions::default();
        let temp_dir = TempDir::new().unwrap();
        let dir_path = temp_dir.path().join("foo");

        fs::create_dir_all(dir_path).unwrap();

        let input = Cli::from_args(&[
            String::from("ja2"),
            String::from("--gamedir"),
            String::from(temp_dir.path().join("foo/../foo/../").to_str().unwrap()),
        ]);

        assert_eq!(
            input.apply_to_engine_options(&mut engine_options).err(),
            None
        );
        assert_eq!(engine_options.vanilla_game_dir, temp_dir.path());
    }

    #[test]
    #[cfg(windows)]
    fn apply_to_engine_options_should_return_the_correct_canonical_game_dir_on_windows() {
        let mut engine_options = EngineOptions::default();
        let temp_dir = TempDir::new().unwrap();
        let dir_path = temp_dir.path().join("foo");

        fs::create_dir_all(dir_path).unwrap();

        let input = Cli::from_args(&[
            String::from("ja2"),
            String::from("--gamedir"),
            String::from(temp_dir.path().to_str().unwrap()),
        ]);

        assert_eq!(
            input.apply_to_engine_options(&mut engine_options).err(),
            None
        );
        assert_eq!(
            engine_options.vanilla_game_dir,
            fs::canonicalize(temp_dir.path()).unwrap()
        );
    }

    #[test]
    fn apply_to_engine_options_should_fail_with_non_existing_directory() {
        let mut engine_options: super::EngineOptions = Default::default();
        let input = Cli::from_args(&[
            String::from("ja2"),
            String::from("--gamedir"),
            String::from("somethingelse"),
        ]);

        assert_eq!(
            input
                .apply_to_engine_options(&mut engine_options)
                .err()
                .unwrap(),
            CliError::GameDirDoesNotExist("somethingelse".to_owned())
        );
    }
}
