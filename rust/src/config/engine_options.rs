use std::path::PathBuf;

use crate::config::Resolution;
use crate::config::ScalingQuality;
use crate::config::VanillaVersion;

/// Struct that is used to store the engines configuration parameters
#[derive(Debug, PartialEq)]
pub struct EngineOptions {
    /// Path to configuration directory in the user's home directory
    pub stracciatella_home: PathBuf,
    /// Path to the vanilla game directory
    pub vanilla_game_dir: PathBuf,
    /// List of enabled mods
    pub mods: Vec<String>,
    /// Resolution the game will start in
    pub resolution: Resolution,
    /// Gamma correction parameter
    pub brightness: f32,
    /// Vanilla game version that the user is in posession of
    pub resource_version: VanillaVersion,
    /// Whether to show help on startup and exit
    pub show_help: bool,
    /// Whether to run unittests on startup and exit
    pub run_unittests: bool,
    /// Wether to run the editor instead of the game itself
    pub run_editor: bool,
    /// Whether to start the game in fullscreen
    pub start_in_fullscreen: bool,
    /// Whether to start the game in windowed mode
    pub start_in_window: bool,
    /// Scaling quality that is used when scaling up game resources
	pub scaling_quality: ScalingQuality,
    /// Whether to start in debug mode
    pub start_in_debug_mode: bool,
    /// Whether to enable sound
    pub start_without_sound: bool,
}

impl Default for EngineOptions {
    fn default() -> EngineOptions {
        EngineOptions {
            stracciatella_home: PathBuf::from(""),
            vanilla_game_dir: PathBuf::from(""),
            mods: vec!(),
            resolution: Resolution::default(),
            brightness: 1.0,
            resource_version: VanillaVersion::ENGLISH,
            show_help: false,
            run_unittests: false,
            run_editor: false,
            start_in_fullscreen: false,
            start_in_window: true,
			scaling_quality: ScalingQuality::default(),
            start_in_debug_mode: false,
            start_without_sound: false,
        }
    }
}

impl EngineOptions {
    /// Construct an EngineOptions instance from home directory and Cli arguments
    ///
    /// Takes Cli arguments and JSON configuration file into account. It will also
    /// create a default JSON configuration file if it does not exist yet.
    pub fn from_home_and_args(stracciatella_home: &PathBuf, args: &[String]) -> Result<EngineOptions, String> {
        use crate::ensure_json_config_existence;
        use crate::parse_json_config;
        use crate::parse_args;

        ensure_json_config_existence(stracciatella_home)?;

        let mut engine_options = parse_json_config(&stracciatella_home)?;

        engine_options.stracciatella_home = stracciatella_home.clone();

        match parse_args(&mut engine_options, args) {
            None => Ok(()),
            Some(str) => Err(str)
        }?;

        if engine_options.vanilla_game_dir == PathBuf::from("") {
            return Err(String::from("Vanilla data directory has to be set either in config file or per command line switch"))
        }

        Ok(engine_options)
    }
}
