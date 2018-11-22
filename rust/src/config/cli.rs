use std::fs;
use std::path::PathBuf;
use std::str::FromStr;

use getopts::Options;

use EngineOptions;
use Resolution;
use VanillaVersion;

#[cfg(not(windows))]
static DATA_DIR_OPTION_EXAMPLE: &'static str = "/opt/ja2";

#[cfg(windows)]
static DATA_DIR_OPTION_EXAMPLE: &'static str = "C:\\JA2";

pub struct Cli {
    args: Vec<String>,
    options: Options
}

impl Cli {
    pub fn from_args(args: &[String]) -> Self {
        let mut opts = Options::new();

        opts.long_only(true);
        opts.optmulti(
            "",
            "datadir",
            "Set path for data directory",
            DATA_DIR_OPTION_EXAMPLE
        );
        opts.optmulti(
            "",
            "mod",
            "Start one of the game modifications. MOD_NAME is the name of modification, e.g. 'from-russia-with-love. See mods folder for possible options'.",
            "MOD_NAME"
        );
        opts.optopt(
            "",
            "res",
            "Screen resolution, e.g. 800x600. Default value is 640x480",
            "WIDTHxHEIGHT"
        );
        opts.optopt(
            "",
            "brightness",
            "Screen brightness (gamma multiplier) value to set where 0.0 is completely dark and 1.0 is normal brightness. Default value is 1.0",
            "GAMMA_VALUE"
        );
        opts.optopt(
            "",
            "resversion",
            "Version of the game resources. Possible values: DUTCH, ENGLISH, FRENCH, GERMAN, ITALIAN, POLISH, RUSSIAN, RUSSIAN_GOLD. Default value is ENGLISH. RUSSIAN is for BUKA Agonia Vlasty release. RUSSIAN_GOLD is for Gold release",
            "RUSSIAN_GOLD"
        );
        opts.optflag(
            "",
            "unittests",
            "Perform unit tests. E.g. 'ja2.exe -unittests --gtest_output=\"xml:report.xml\" --gtest_repeat=2'");
        opts.optflag(
            "",
            "editor",
            "Start the map editor (Editor.slf is required)"
        );
        opts.optflag(
            "",
            "fullscreen",
            "Start the game in the fullscreen mode"
        );
        opts.optflag(
            "",
            "nosound",
            "Turn the sound and music off"
        );
        opts.optflag(
            "",
            "window",
            "Start the game in a window"
        );
        opts.optflag(
            "",
            "debug",
            "Enable Debug Mode"
        );
        opts.optflag(
            "",
            "help",
            "print this help menu"
        );

        Cli {
            args: args.to_vec(),
            options: opts
        }
    }

    pub fn apply_to_engine_options(&self, engine_options: &mut EngineOptions) -> Result<(), String> {
        match self.options.parse(&self.args[1..]) {
            Ok(m) => {
                if !m.free.is_empty() {
                    return Err(format!("Unknown arguments: '{}'.", m.free.join(" ")));
                }

                if let Some(s) = m.opt_str("datadir") {
                    match fs::canonicalize(PathBuf::from(s)) {
                        Ok(s) => {
                            let mut temp = String::from(s.to_str().expect("Should not happen"));
                            // remove UNC path prefix (Windows)
                            if temp.starts_with("\\\\") {
                                temp.drain(..2);
                                let pos = temp.find('\\').unwrap() + 1;
                                temp.drain(..pos);
                            }
                            engine_options.vanilla_data_dir = PathBuf::from(temp)
                        },
                        Err(_) => return Err(String::from("Please specify an existing datadir."))
                    };
                }

                if !m.opt_strs("mod").is_empty() {
                    engine_options.mods = m.opt_strs("mod");
                }

                if let Some(s) = m.opt_str("res") {
                    match Resolution::from_str(&s) {
                        Ok(res) => {
                            engine_options.resolution = res;
                        },
                        Err(s) => return Err(s)
                    }
                }

                if let Some(s) = m.opt_str("brightness") {
                    match s.parse::<f32>() {
                        Ok(val) => {
                            engine_options.brightness = val;
                        },
                        Err(_e) => return Err(String::from("Incorrect brightness value."))
                    }
                }

                if let Some(s) = m.opt_str("resversion") {
                    match VanillaVersion::from_str(&s) {
                        Ok(resource_version) => {
                            engine_options.resource_version = resource_version
                        },
                        Err(s) => return Err(s)
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
                }

                if m.opt_present("nosound") {
                    engine_options.start_without_sound = true;
                }

                if m.opt_present("window") {
                    engine_options.start_in_window = true;
                }

                if m.opt_present("debug") {
                    engine_options.start_in_debug_mode = true;
                }

                Ok(())
            }
            Err(f) => Err(f.to_string())
        }
    }

    pub fn usage() -> String {
        let cli = Cli::from_args(&[]);
        cli.options.usage("Usage: ja2 [options]")
    }
}