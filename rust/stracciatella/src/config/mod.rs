//! This module contains code to configure the ja2-stracciatella engine

mod cli;
mod engine_options;
mod ja2_json;
mod resolution;
mod scaling_quality;
mod stracciatella_home;
mod vanilla_version;

pub use self::cli::{Cli, CliError};
pub use self::engine_options::{EngineOptions, EngineOptionsError};
pub use self::ja2_json::{Ja2Json, Ja2JsonError};
pub use self::resolution::Resolution;
pub use self::scaling_quality::ScalingQuality;
pub use self::stracciatella_home::find_stracciatella_home;
pub use self::vanilla_version::VanillaVersion;
