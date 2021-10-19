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

#[derive(Debug, PartialEq, Serialize, Deserialize)]
pub struct Ja2JsonContent {
    #[serde(skip_serializing)]
    data_dir: Option<PathBuf>,
    game_dir: Option<PathBuf>,
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

fn build_json_config_location(stracciatella_home: &PathBuf) -> PathBuf {
    resolve_existing_components(&Path::new("ja2.json"), Some(&stracciatella_home), true)
}

impl Ja2Json {
    /// Construct a Ja2Json instance from the stracciatella home directory
    pub fn from_stracciatella_home(stracciatella_home: &PathBuf) -> Self {
        let path = build_json_config_location(stracciatella_home);
        Ja2Json { path }
    }

    fn get_content(&self) -> Result<Ja2JsonContent, String> {
        let s = fs::read_to_string(&self.path)
            .map_err(|x| format!("Error reading ja2.json config file: {}", x.to_string()))?;
        json::de::from_string(&s).map_err(|x| format!("Error parsing ja2.json config file: {}", x))
    }

    /// Apply current JSON file contents to EngineOptions struct
    pub fn apply_to_engine_options(
        &self,
        engine_options: &mut EngineOptions,
    ) -> Result<(), String> {
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
        engine_options.vanilla_game_dir =
            resolve_existing_components(&engine_options.vanilla_game_dir, None, true);
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
    pub fn ensure_existence(&self) -> Result<(), String> {
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
                fs::create_dir_all(p)
                    .map_err(|why| format!("Error creating {:?}: {:?}", p, why.kind()))?;
            }
        }

        if !self.path.exists() {
            let mut f = File::create(&self.path)
                .map_err(|why| format!("Error creating {:?}: {:?}", self.path, why.kind()))?;
            f.write_all(DEFAULT_JSON_CONTENT.as_bytes())
                .map_err(|why| format!("Error writing {:?}: {:?}", self.path, why.kind()))?;
        }

        Ok(())
    }
}
