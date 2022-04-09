use std::fs;
use std::fs::File;
use std::io::Write;
use std::path::PathBuf;

use log::warn;
use serde::{Deserialize, Serialize};
use std::path::Path;

use crate::config::{EngineOptions, Resolution, ScalingQuality, VanillaVersion};
use crate::fs::resolve_existing_components;
use crate::json;

#[derive(Debug, Clone, PartialEq)]
pub enum Ja2JsonError {
    CreatingFailed(String),
    ReadingFailed(String),
    ParsingFailed(String),
}

impl std::fmt::Display for Ja2JsonError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::CreatingFailed(e) => write!(f, "Error creating ja2.json config file: {}", e),
            Self::ReadingFailed(e) => write!(f, "Error reading ja2.json config file: {}", e),
            Self::ParsingFailed(e) => write!(f, "Error parsing ja2.json config file: {}", e),
        }
    }
}

impl std::error::Error for Ja2JsonError {}

#[derive(Debug, PartialEq, Serialize, Deserialize)]
pub struct Ja2JsonContent {
    #[serde(skip_serializing)]
    data_dir: Option<PathBuf>,
    game_dir: Option<PathBuf>,
    save_game_dir: Option<PathBuf>,
    mods: Option<Vec<String>>,
    res: Option<Resolution>,
    brightness: Option<f32>,
    resversion: Option<VanillaVersion>,
    fullscreen: Option<bool>,
    scaling: Option<ScalingQuality>,
    debug: Option<bool>,
    nosound: Option<bool>,
}

/// Struct to handle interactions with the JSON configuration file
pub struct Ja2Json {
    path: PathBuf,
}

fn build_json_config_location(stracciatella_home: &Path) -> PathBuf {
    resolve_existing_components(&Path::new("ja2.json"), Some(stracciatella_home), true)
}

impl Ja2Json {
    /// Construct a Ja2Json instance from the stracciatella home directory
    pub fn from_stracciatella_home<P>(stracciatella_home: P) -> Self
    where
        P: AsRef<Path>,
    {
        let path = build_json_config_location(stracciatella_home.as_ref());
        Ja2Json { path }
    }

    fn get_content(&self) -> Result<Ja2JsonContent, Ja2JsonError> {
        let s = fs::read_to_string(&self.path)
            .map_err(|x| Ja2JsonError::ReadingFailed(x.to_string()))?;
        json::de::from_string(&s).map_err(Ja2JsonError::ParsingFailed)
    }

    /// Apply current JSON file contents to EngineOptions struct
    pub fn apply_to_engine_options(
        &self,
        engine_options: &mut EngineOptions,
    ) -> Result<(), Ja2JsonError> {
        macro_rules! copy_to {
            ($from: expr, $to: expr) => {
                if let Some(v) = $from {
                    $to = v;
                }
            };
        }

        let content = self.get_content()?;

        if let Some(data_dir) = content.data_dir {
            warn!("`data_dir` option in ja2.json is deprecated, use `game_dir` instead");
            engine_options.vanilla_game_dir = data_dir;
        }

        copy_to!(content.game_dir, engine_options.vanilla_game_dir);
        copy_to!(content.save_game_dir, engine_options.save_game_dir);
        copy_to!(
            content
                .mods
                .map(|c| c.iter().map(|v| v.to_lowercase()).collect()),
            engine_options.mods
        );
        copy_to!(content.res, engine_options.resolution);
        copy_to!(content.brightness, engine_options.brightness);
        copy_to!(content.resversion, engine_options.resource_version);
        copy_to!(content.fullscreen, engine_options.start_in_fullscreen);
        copy_to!(content.scaling, engine_options.scaling_quality);
        copy_to!(content.debug, engine_options.start_in_debug_mode);
        copy_to!(content.nosound, engine_options.start_without_sound);

        Ok(())
    }

    /// Write current contents of EngineOptions to JSON configuration file
    pub fn write(&self, engine_options: &EngineOptions) -> Result<(), String> {
        macro_rules! copy_to {
            ($from: expr, $to: expr) => {
                $to = Some($from.clone());
            };
        }

        let mut content = Ja2JsonContent {
            data_dir: None,
            game_dir: None,
            save_game_dir: None,
            mods: None,
            res: None,
            brightness: None,
            resversion: None,
            fullscreen: None,
            scaling: None,
            debug: None,
            nosound: None,
        };

        copy_to!(engine_options.vanilla_game_dir, content.game_dir);
        copy_to!(engine_options.save_game_dir, content.save_game_dir);
        copy_to!(engine_options.mods, content.mods);
        copy_to!(engine_options.resolution, content.res);
        copy_to!(engine_options.brightness, content.brightness);
        copy_to!(engine_options.resource_version, content.resversion);
        copy_to!(engine_options.start_in_fullscreen, content.fullscreen);
        copy_to!(engine_options.scaling_quality, content.scaling);
        copy_to!(engine_options.start_in_debug_mode, content.debug);
        copy_to!(engine_options.start_without_sound, content.nosound);

        let json = json::ser::to_string(&content)
            .map_err(|x| format!("Error creating contents of ja2.json config file: {}", x))?;
        let mut f = File::create(&self.path)
            .map_err(|s| format!("Error creating ja2.json config file: {}", s.to_string()))?;

        f.write_all(json.as_bytes())
            .map_err(|s| format!("Error creating ja2.json config file: {}", s.to_string()))
    }

    /// Ensures that the JSON configuration file exists and write a default one if it doesn't
    pub fn ensure_existence(&self) -> Result<(), Ja2JsonError> {
        #[cfg(not(windows))]
        static DEFAULT_JSON_CONTENT: &str = r##"{
            // Put the directory to your original ja2 installation into the line below.
            "game_dir": "/some/place/where/the/data/is"
        }"##;
        #[cfg(windows)]
        static DEFAULT_JSON_CONTENT: &str = r##"{
            // Put the directory to your original ja2 installation into the line below. Make sure to use double backslashes.
            "game_dir": "C:\\Program Files\\Jagged Alliance 2"
        }"##;

        let parent = self.path.parent();
        if let Some(p) = parent {
            if !p.exists() {
                fs::create_dir_all(p).map_err(|why| {
                    Ja2JsonError::CreatingFailed(format!(
                        "Error creating directory {:?}: {:?}",
                        p,
                        why.to_string()
                    ))
                })?;
            }
        }

        if !self.path.exists() {
            let mut f = File::create(&self.path).map_err(|why| {
                Ja2JsonError::CreatingFailed(format!(
                    "Error creating file {:?}: {:?}",
                    self.path,
                    why.to_string()
                ))
            })?;
            f.write_all(DEFAULT_JSON_CONTENT.as_bytes())
                .map_err(|why| {
                    Ja2JsonError::CreatingFailed(format!(
                        "Error writing file {:?}: {:?}",
                        self.path,
                        why.to_string()
                    ))
                })?;
        }

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use std::io::prelude::*;
    use std::path::Path;

    use tempfile::TempDir;

    use super::*;
    use crate::config::VanillaVersion;
    use crate::fs;
    use crate::fs::File;

    pub fn write_temp_folder_with_ja2_json(contents: &[u8]) -> TempDir {
        let dir = TempDir::new().unwrap();
        let ja2_home_dir = dir.path().join(".ja2");
        let file_path = ja2_home_dir.join("ja2.json");

        fs::create_dir(ja2_home_dir).unwrap();
        std::fs::write(file_path, contents).unwrap();

        dir
    }

    #[test]
    fn ensure_existence_should_ensure_existence_of_config_dir() {
        let dir = TempDir::new().unwrap();
        let home_path = dir.path().join("ja2_home");
        let expected_path = home_path.join("ja2.json");

        let ja2json = Ja2Json::from_stracciatella_home(&home_path);

        ja2json.ensure_existence().unwrap();

        assert!(home_path.exists());
        assert!(expected_path.is_file());
    }

    #[test]
    fn ensure_existence_should_not_overwrite_existing_ja2json() {
        let dir = write_temp_folder_with_ja2_json(b"Test");
        let ja2json = Ja2Json::from_stracciatella_home(dir.path().join(".ja2"));
        let ja2json_path = dir.path().join(".ja2/ja2.json");

        ja2json.ensure_existence().unwrap();

        let mut f = File::open(ja2json_path.clone()).unwrap();
        let mut content: Vec<u8> = vec![];
        f.read_to_end(&mut content).unwrap();

        assert!(ja2json_path.is_file());
        assert_eq!(content, b"Test");
    }

    #[test]
    fn apply_to_engine_options_should_fail_with_missing_file() {
        let mut engine_options = EngineOptions::default();
        let temp_dir = TempDir::new().unwrap();
        let ja2json = Ja2Json::from_stracciatella_home(temp_dir.path().join(".ja2"));
        let result = ja2json.apply_to_engine_options(&mut engine_options);

        match result {
            Err(Ja2JsonError::ReadingFailed(_)) => {}
            _ => panic!("incorrect error variant"),
        }
    }

    #[test]
    fn apply_to_engine_options_should_fail_with_invalid_json() {
        let mut engine_options = EngineOptions::default();
        let temp_dir = write_temp_folder_with_ja2_json(b"{ not json }");
        let ja2json = Ja2Json::from_stracciatella_home(temp_dir.path().join(".ja2"));
        let result = ja2json.apply_to_engine_options(&mut engine_options);

        match result {
            Err(Ja2JsonError::ParsingFailed(_)) => {}
            _ => panic!("incorrect error variant"),
        }
    }

    #[test]
    fn apply_to_engine_options_should_be_able_to_change_game_dir() {
        let mut engine_options = EngineOptions::default();
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"game_dir\": \"/dd\" }");
        let ja2json = Ja2Json::from_stracciatella_home(temp_dir.path().join(".ja2"));

        ja2json
            .apply_to_engine_options(&mut engine_options)
            .unwrap();

        assert_eq!(engine_options.vanilla_game_dir, Path::new("/dd"));
    }

    #[test]
    fn apply_to_engine_options_should_be_able_to_change_game_dir_with_data_dir() {
        let mut engine_options = EngineOptions::default();
        // data_dir is an alias to game_dir
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"data_dir\": \"/dd\" }");
        let ja2json = Ja2Json::from_stracciatella_home(temp_dir.path().join(".ja2"));

        ja2json
            .apply_to_engine_options(&mut engine_options)
            .unwrap();

        assert_eq!(engine_options.vanilla_game_dir, Path::new("/dd"));
    }

    #[test]
    fn apply_to_engine_options_should_be_able_to_change_fullscreen_value() {
        let mut engine_options = EngineOptions::default();
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"fullscreen\": true }");
        let ja2json = Ja2Json::from_stracciatella_home(temp_dir.path().join(".ja2"));

        ja2json
            .apply_to_engine_options(&mut engine_options)
            .unwrap();

        assert_eq!(engine_options.start_in_fullscreen, true);
    }

    #[test]
    fn apply_to_engine_options_should_be_able_to_change_debug_value() {
        let mut engine_options = EngineOptions::default();
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"debug\": true }");
        let ja2json = Ja2Json::from_stracciatella_home(temp_dir.path().join(".ja2"));

        ja2json
            .apply_to_engine_options(&mut engine_options)
            .unwrap();

        assert_eq!(engine_options.start_in_debug_mode, true);
    }

    #[test]
    fn apply_to_engine_options_should_be_able_to_start_without_sound() {
        let mut engine_options = EngineOptions::default();
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"nosound\": true }");
        let ja2json = Ja2Json::from_stracciatella_home(temp_dir.path().join(".ja2"));

        ja2json
            .apply_to_engine_options(&mut engine_options)
            .unwrap();

        assert_eq!(engine_options.start_without_sound, true);
    }

    #[test]
    fn apply_to_engine_options_should_not_be_able_to_run_help() {
        let mut engine_options = EngineOptions::default();
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"help\": true, \"show_help\": true }");
        let ja2json = Ja2Json::from_stracciatella_home(temp_dir.path().join(".ja2"));

        ja2json
            .apply_to_engine_options(&mut engine_options)
            .unwrap();

        assert_eq!(engine_options.show_help, false);
    }

    #[test]
    fn apply_to_engine_options_should_not_be_able_to_run_unittests() {
        let mut engine_options = EngineOptions::default();
        let temp_dir =
            write_temp_folder_with_ja2_json(b"{ \"unittests\": true, \"run_unittests\": true }");
        let ja2json = Ja2Json::from_stracciatella_home(temp_dir.path().join(".ja2"));

        ja2json
            .apply_to_engine_options(&mut engine_options)
            .unwrap();

        assert_eq!(engine_options.run_unittests, false);
    }

    #[test]
    fn apply_to_engine_options_should_not_be_able_to_run_editor() {
        let mut engine_options = EngineOptions::default();
        let temp_dir =
            write_temp_folder_with_ja2_json(b"{ \"editor\": true, \"run_editor\": true }");
        let ja2json = Ja2Json::from_stracciatella_home(temp_dir.path().join(".ja2"));

        ja2json
            .apply_to_engine_options(&mut engine_options)
            .unwrap();

        assert_eq!(engine_options.run_editor, false);
    }

    #[test]
    fn apply_to_engine_options_should_fail_with_invalid_mod() {
        let mut engine_options = EngineOptions::default();
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"mods\": [ \"a\", true ] }");
        let ja2json = Ja2Json::from_stracciatella_home(temp_dir.path().join(".ja2"));
        let result = ja2json.apply_to_engine_options(&mut engine_options);

        match result {
            Err(Ja2JsonError::ParsingFailed(_)) => {}
            _ => panic!("incorrect error variant"),
        }
    }

    #[test]
    fn apply_to_engine_options_should_continue_with_multiple_known_switches() {
        let mut engine_options = EngineOptions::default();
        let temp_dir =
            write_temp_folder_with_ja2_json(b"{ \"debug\": true, \"mods\": [ \"m1\", \"a2\" ] }");
        let ja2json = Ja2Json::from_stracciatella_home(temp_dir.path().join(".ja2"));

        ja2json
            .apply_to_engine_options(&mut engine_options)
            .unwrap();

        assert_eq!(engine_options.start_in_debug_mode, true);
        assert_eq!(engine_options.mods.len(), 2);
    }

    #[test]
    fn apply_to_engine_options_should_fail_with_unknown_resversion() {
        let mut engine_options = EngineOptions::default();
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"resversion\": \"TESTUNKNOWN\" }");
        let ja2json = Ja2Json::from_stracciatella_home(temp_dir.path().join(".ja2"));
        let result = ja2json.apply_to_engine_options(&mut engine_options);

        match result {
            Err(Ja2JsonError::ParsingFailed(_)) => {}
            _ => panic!("incorrect error variant"),
        }
    }

    #[test]
    fn apply_to_engine_options_should_return_the_correct_resversion_for_russian() {
        let mut engine_options = EngineOptions::default();
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"resversion\": \"RUSSIAN\" }");
        let ja2json = Ja2Json::from_stracciatella_home(temp_dir.path().join(".ja2"));

        ja2json
            .apply_to_engine_options(&mut engine_options)
            .unwrap();

        assert_eq!(engine_options.resource_version, VanillaVersion::RUSSIAN);
    }

    #[test]
    fn apply_to_engine_options_should_return_the_correct_resversion_for_italian() {
        let mut engine_options = EngineOptions::default();
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"resversion\": \"ITALIAN\" }");
        let ja2json = Ja2Json::from_stracciatella_home(temp_dir.path().join(".ja2"));

        ja2json
            .apply_to_engine_options(&mut engine_options)
            .unwrap();

        assert_eq!(engine_options.resource_version, VanillaVersion::ITALIAN);
    }

    #[test]
    fn apply_to_engine_options_should_return_the_correct_resolution() {
        let mut engine_options = EngineOptions::default();
        let temp_dir = write_temp_folder_with_ja2_json(b"{ \"res\": \"1024x768\" }");
        let ja2json = Ja2Json::from_stracciatella_home(temp_dir.path().join(".ja2"));

        ja2json
            .apply_to_engine_options(&mut engine_options)
            .unwrap();

        assert_eq!(engine_options.resolution.0, 1024);
        assert_eq!(engine_options.resolution.1, 768);
    }
}
