use std::path::{Path, PathBuf};

use crate::config::{Resolution, ScalingQuality, VanillaVersion};
use crate::fs::resolve_existing_components;
use crate::{ensure_json_config_existence, get_assets_dir, parse_args, parse_json_config};

pub const SAVED_GAME_DIR: &str = "SavedGames";

/// Struct that is used to store the engines configuration parameters
#[derive(Debug, PartialEq)]
pub struct EngineOptions {
    /// Path to configuration directory in the user's home directory
    pub stracciatella_home: PathBuf,
    /// Path to the vanilla game directory
    pub vanilla_game_dir: PathBuf,
    /// Path to the assets directory included with Stracciatella
    pub assets_dir: PathBuf,
    /// Path to the save game directory (usually within stracciatella_home)
    pub save_game_dir: PathBuf,
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
            assets_dir: PathBuf::from(""),
            save_game_dir: PathBuf::from(""),
            mods: vec![],
            resolution: Resolution::default(),
            brightness: -1.0,
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
    pub fn from_home_and_args(
        stracciatella_home: &PathBuf,
        args: &[String],
    ) -> Result<EngineOptions, String> {
        ensure_json_config_existence(stracciatella_home)?;

        let mut engine_options = parse_json_config(&stracciatella_home)?;

        match parse_args(&mut engine_options, args) {
            None => Ok(()),
            Some(str) => Err(str),
        }?;

        if engine_options.vanilla_game_dir == PathBuf::from("") {
            return Err(String::from("Vanilla data directory has to be set either in config file or per command line switch"));
        }

        engine_options.stracciatella_home =
            resolve_existing_components(stracciatella_home, None, true);
        engine_options.assets_dir = resolve_existing_components(&get_assets_dir(), None, true);
        engine_options.vanilla_game_dir =
            resolve_existing_components(&engine_options.vanilla_game_dir, None, true);

        engine_options.ensure_save_game_directory()?;

        Ok(engine_options)
    }

    /// Sets the save game folder to default if it is not set and ensures it exists
    fn ensure_save_game_directory(&mut self) -> Result<(), String> {
        let default_save_game_dir = resolve_existing_components(
            &Path::new(SAVED_GAME_DIR),
            Some(&self.stracciatella_home),
            true,
        );

        if self.save_game_dir == Path::new("") && self.stracciatella_home != Path::new("") {
            self.save_game_dir = default_save_game_dir.clone()
        }

        if self.save_game_dir == default_save_game_dir {
            if !self.save_game_dir.exists() {
                std::fs::create_dir(&self.save_game_dir).map_err(|e| {
                    format!(
                        "Error creating default save game dir `{:?}`: {}",
                        &default_save_game_dir, e
                    )
                })?;
            }
        } else {
            self.save_game_dir = resolve_existing_components(&self.save_game_dir, None, true);
        }

        Ok(())
    }

    /// Checks whether a specific mod is enabled
    pub fn is_mod_enabled(&self, name: &str) -> bool {
        self.mods.contains(&name.to_lowercase())
    }
}
