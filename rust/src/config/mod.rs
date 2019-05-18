//! This module contains code to configure the ja2-stracciatella engine

mod scaling_quality;
mod vanilla_version;
mod resolution;
mod ja2_json;
mod cli;
mod stracciatella_home;

mod engine_options;

pub use self::scaling_quality::ScalingQuality;
pub use self::vanilla_version::VanillaVersion;
pub use self::resolution::Resolution;
pub use self::ja2_json::Ja2Json;
pub use self::cli::Cli;
pub use self::engine_options::EngineOptions;
pub use self::stracciatella_home::find_stracciatella_home;
