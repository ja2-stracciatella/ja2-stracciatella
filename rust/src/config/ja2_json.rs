use std::error::Error;
use std::fs;
use std::fs::File;
use std::io::Write;
use std::path::PathBuf;

use serde::Deserialize;
use serde::Serialize;

use crate::Resolution;
use crate::VanillaVersion;
use crate::ScalingQuality;
use crate::EngineOptions;

#[derive(Debug, PartialEq, Serialize, Deserialize)]
pub struct Ja2JsonContent {
    data_dir: Option<PathBuf>,
    mods: Option<Vec<String>>,
    res: Option<Resolution>,
    brightness: Option<f32>,
    resversion: Option<VanillaVersion>,
    fullscreen: Option<bool>,
    scaling: Option<ScalingQuality>,
    debug: Option<bool>,
    nosound: Option<bool>,
}

pub struct Ja2Json {
    path: PathBuf
}

fn build_json_config_location(stracciatella_home: &PathBuf) -> PathBuf {
    let mut path = PathBuf::from(stracciatella_home);
    path.push("ja2.json");
    path
}

impl Ja2Json {
    pub fn from_stracciatella_home(stracciatella_home: &PathBuf) -> Self {
        let path = build_json_config_location(stracciatella_home);
        Ja2Json { path }
    }

    pub fn get_content(&self) -> Result<Ja2JsonContent, String> {
        File::open(&self.path)
            .map_err(|s| format!("Error reading ja2.json config file: {}", s.description()))
            .and_then(|f| serde_json::from_reader(f).map_err(|s| format!("Error parsing ja2.json config file: {}", s)))
    }

    pub fn apply_to_engine_options(&self, engine_options: &mut EngineOptions) -> Result<(), String> {
        macro_rules! copy_to {
            ($from: expr, $to: expr) => { if let Some(v) = $from { $to = v; } }
        }

        let content = self.get_content()?;

        copy_to!(content.data_dir, engine_options.vanilla_data_dir);
        copy_to!(content.mods, engine_options.mods);
        copy_to!(content.res, engine_options.resolution);
        copy_to!(content.brightness, engine_options.brightness);
        copy_to!(content.resversion, engine_options.resource_version);
        copy_to!(content.fullscreen, engine_options.start_in_fullscreen);
        copy_to!(content.scaling, engine_options.scaling_quality);
        copy_to!(content.debug, engine_options.start_in_debug_mode);
        copy_to!(content.nosound, engine_options.start_without_sound);

        Ok(())
    }

    pub fn write(&self, engine_options: &EngineOptions) -> Result<(), String> {
        macro_rules! copy_to {
            ($from: expr, $to: expr) => { $to = Some($from.clone()); }
        }

        let mut content = Ja2JsonContent {
            data_dir: None,
            mods: None,
            res: None,
            brightness: None,
            resversion: None,
            fullscreen: None,
            scaling: None,
            debug: None,
            nosound: None,
        };

        copy_to!(engine_options.vanilla_data_dir, content.data_dir);
        copy_to!(engine_options.mods, content.mods);
        copy_to!(engine_options.resolution, content.res);
        copy_to!(engine_options.brightness, content.brightness);
        copy_to!(engine_options.resource_version, content.resversion);
        copy_to!(engine_options.start_in_fullscreen, content.fullscreen);
        copy_to!(engine_options.scaling_quality, content.scaling);
        copy_to!(engine_options.start_in_debug_mode, content.debug);
        copy_to!(engine_options.start_without_sound, content.nosound);


        let json = serde_json::to_string_pretty(&content).map_err(|s| format!("Error creating contents of ja2.json config file: {}", s.description()))?;
        let mut f = File::create(&self.path).map_err(|s| format!("Error creating ja2.json config file: {}", s.description()))?;

        f.write_all(json.as_bytes()).map_err(|s| format!("Error creating ja2.json config file: {}", s.description()))
    }

    pub fn ensure_existence(&self) -> Result<(), String> {
        #[cfg(not(windows))]
        static DEFAULT_JSON_CONTENT: &'static str = r##"{
            "help": "Put the directory to your original ja2 installation into the line below",
            "data_dir": "/some/place/where/the/data/is"
        }"##;
        #[cfg(windows)]
        static DEFAULT_JSON_CONTENT: &'static str = r##"{
            "help": "Put the directory to your original ja2 installation into the line below. Make sure to use double backslashes.",
            "data_dir": "C:\\Program Files\\Jagged Alliance 2"
        }"##;

        let parent = self.path.parent();
        if let Some(p) = parent {
            if !p.exists() {
                fs::create_dir_all(p).map_err(|why| format!("Error creating {:?}: {:?}", p, why.kind()))?;
            }
        }

        if !self.path.exists() {
            let mut f = File::create(&self.path).map_err(|why| format!("Error creating {:?}: {:?}", self.path, why.kind()))?;
            f.write_all(DEFAULT_JSON_CONTENT.as_bytes()).map_err(|why| format!("Error writing {:?}: {:?}", self.path, why.kind()))?;
        }

        Ok(())
    }
}
