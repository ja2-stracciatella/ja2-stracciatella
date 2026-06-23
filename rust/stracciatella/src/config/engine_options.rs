use std::path::{Path, PathBuf};

use crate::config::{Resolution, ScalingQuality, VanillaVersion};
use crate::fs::resolve_existing_components;
use crate::get_assets_dir;

use super::{Cli, CliError, Ja2Json, Ja2JsonError};

pub const SAVED_GAME_DIR: &str = "SavedGames";
const STRACCIATELLA_HOME_DIR_NAME: &str = "/ja2";

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum EngineOptionsError {
    Cli(CliError),
    Ja2Json(Ja2JsonError),
    MissingGameDir,
    CreatingDefaultSaveGameDirFailed(PathBuf, String),
}

impl std::fmt::Display for EngineOptionsError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            EngineOptionsError::Cli(e) => write!(f, "Error evaluating CLI arguments: {}", e),
            EngineOptionsError::Ja2Json(e) => write!(f, "Error loading JSON configuration: {}", e),
            EngineOptionsError::MissingGameDir => write!(
                f,
                "Vanilla game directory has to be set either in config file or per command line switch"
            ),
            EngineOptionsError::CreatingDefaultSaveGameDirFailed(path, e) => write!(
                f,
                "Error creating default save game dir `{:?}`: {}",
                path, e
            ),
        }
    }
}

impl std::error::Error for EngineOptionsError {}

impl From<CliError> for EngineOptionsError {
    fn from(c: CliError) -> Self {
        EngineOptionsError::Cli(c)
    }
}

impl From<Ja2JsonError> for EngineOptionsError {
    fn from(c: Ja2JsonError) -> Self {
        EngineOptionsError::Ja2Json(c)
    }
}

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
    /// Whether to enum-gen for Lua
    pub run_enum_gen: bool,
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
            run_enum_gen: false,
        }
    }
}

impl EngineOptions {
    /// Construct an EngineOptions instance from home directory and Cli arguments
    ///
    /// Takes Cli arguments and JSON configuration file into account. It will also
    /// create a default JSON configuration file if it does not exist yet.
    pub fn from_home_and_args(
        stracciatella_home: &Path,
        args: &[String],
    ) -> Result<EngineOptions, EngineOptionsError> {
        let mut engine_options = EngineOptions::default();
        let cli = Cli::from_args(args);
        let ja2_json = Ja2Json::from_stracciatella_home(stracciatella_home);

        ja2_json.ensure_existence()?;
        ja2_json.apply_to_engine_options(&mut engine_options)?;
        cli.apply_to_engine_options(&mut engine_options)?;

        if engine_options.vanilla_game_dir == PathBuf::from("") {
            return Err(EngineOptionsError::MissingGameDir);
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
    fn ensure_save_game_directory(&mut self) -> Result<(), EngineOptionsError> {
        let mut base = String::new();
	if !Path::new(&format!("{}/{}", std::env::var("HOME").unwrap(), STRACCIATELLA_HOME_DIR_NAME)).is_dir() {
		match std::env::var("XDG_DATA_HOME") {
			Ok(val) => base = val,
			Err(_error) => base = format!("{}/.local/share", std::env::var("HOME").unwrap()),
			_ => todo!(),
		}
		base.push_str(STRACCIATELLA_HOME_DIR_NAME);
	}
	
	let default_save_game_dir = resolve_existing_components(
            Path::new(SAVED_GAME_DIR),
            Some(Path::new(&base)),
            true,
        );

        if self.save_game_dir == Path::new("") && self.stracciatella_home != Path::new("") {
            self.save_game_dir = default_save_game_dir.clone()
        }

        if self.save_game_dir == default_save_game_dir {
            if !self.save_game_dir.exists() {
                std::fs::create_dir_all(&self.save_game_dir).map_err(|e| {
                    EngineOptionsError::CreatingDefaultSaveGameDirFailed(
                        self.save_game_dir.clone(),
                        e.to_string(),
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

#[cfg(test)]
mod tests {
    use std::io::prelude::*;

    use tempfile::TempDir;

    use super::*;
    use crate::fs;
    use crate::fs::File;

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
        assert!(engine_options.start_in_fullscreen);
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
        let engine_options_res = EngineOptions::from_home_and_args(&home, &args);

        assert_eq!(engine_options_res, Err(EngineOptionsError::MissingGameDir));
    }

    #[test]
    fn build_engine_options_from_home_and_args_should_ensure_default_save_game_dir() {
        let temp_dir =
            write_temp_folder_with_ja2_json(b"{ \"game_dir\": \"/some/place/where/the/data/is\", \"res\": \"1024x768\", \"fullscreen\": true }");
        let args = vec![String::from("ja2")];
        let home = temp_dir.path().join(".ja2");

        let engine_options = EngineOptions::from_home_and_args(&home, &args).unwrap();
        let mut expected_vec: Vec<PathBuf> = Vec::new();
	    let expected_save_game_dir1 = temp_dir.path().join(".ja2/SavedGames");
	    let expected_save_game_dir2 = PathBuf::from(format!("{}{}{}", std::env::var("XDG_DATA_HOME").unwrap(), STRACCIATELLA_HOME_DIR_NAME,
	    "/SavedGames"));
	    let expected_save_game_dir3 = PathBuf::from(format!("{}{}{}{}", std::env::var("HOME").unwrap(), "/.local/share", STRACCIATELLA_HOME_DIR_NAME,
	    "/SavedGames"));
	    expected_vec.push(expected_save_game_dir1);
	    expected_vec.push(expected_save_game_dir2);
	    expected_vec.push(expected_save_game_dir3);
	    assert!(expected_vec.contains(&engine_options.save_game_dir));
        assert!(expected_vec[0].is_dir() || expected_vec[1].is_dir() || expected_vec[2].is_dir());
    }

    #[test]
    fn build_engine_options_from_home_and_args_should_use_custom_save_game_dir() {
        let save_temp_dir = TempDir::new().unwrap();
        let ja2_json = format!(
            "{{ \"save_game_dir\": {}, \"game_dir\": \"/some/place/where/the/data/is\", \"res\": \"1024x768\", \"fullscreen\": true }}",
            serde_json::to_string(&save_temp_dir.path().join("saves").to_string_lossy()).unwrap()
        );
        let temp_dir = write_temp_folder_with_ja2_json(ja2_json.as_bytes());
        let args = vec![String::from("ja2")];
        let home = temp_dir.path().join(".ja2");
        let save_dir = save_temp_dir.path().join("Saves");

        std::fs::create_dir(&save_dir).unwrap();
        std::fs::write(save_dir.join("testfile"), "").unwrap();

        let engine_options = EngineOptions::from_home_and_args(&home, &args).unwrap();

        assert!(engine_options.save_game_dir.is_dir());
        assert!(engine_options.save_game_dir.join("testfile").exists())
    }
}
