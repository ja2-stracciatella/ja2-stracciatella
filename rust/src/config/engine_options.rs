use std::path::PathBuf;

use Resolution;
use ScalingQuality;
use VanillaVersion;

#[derive(Debug, PartialEq)]
pub struct EngineOptions {
    pub stracciatella_home: PathBuf,
    pub vanilla_data_dir: PathBuf,
    pub mods: Vec<String>,
    pub resolution: Resolution,
    pub brightness: f32,
    pub resource_version: VanillaVersion,
    pub show_help: bool,
    pub run_unittests: bool,
    pub run_editor: bool,
    pub start_in_fullscreen: bool,
    pub start_in_window: bool,
	pub scaling_quality: ScalingQuality,
    pub start_in_debug_mode: bool,
    pub start_without_sound: bool,
}

impl Default for EngineOptions {
    fn default() -> EngineOptions {
        EngineOptions {
            stracciatella_home: PathBuf::from(""),
            vanilla_data_dir: PathBuf::from(""),
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
    pub fn from_home_and_args(stracciatella_home: &PathBuf, args: &[String]) -> Result<EngineOptions, String> {
        use ensure_json_config_existence;
        use parse_json_config;
        use parse_args;

        ensure_json_config_existence(stracciatella_home)?;

        let mut engine_options = parse_json_config(&stracciatella_home)?;

        engine_options.stracciatella_home = stracciatella_home.clone();

        match parse_args(&mut engine_options, args) {
            None => Ok(()),
            Some(str) => Err(str)
        }?;

        if engine_options.vanilla_data_dir == PathBuf::from("") {
            return Err(String::from("Vanilla data directory has to be set either in config file or per command line switch"))
        }

        Ok(engine_options)
    }
}