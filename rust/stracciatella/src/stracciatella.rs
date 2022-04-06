//! This crate contains the rust code of ja2-stracciatella.
//!
//! The crate [stracciatella_c_api] contains the C API of this crate.
//!
//! [stracciatella_c_api]: ../stracciatella_c_api/index.html

#[cfg(target_os = "android")]
pub mod android;
pub mod config;
pub mod file_formats;
pub mod fs;
pub mod guess;
pub mod json;
pub mod logger;
pub mod math;
pub mod mods;
pub mod res;
pub mod schemas;
pub mod unicode;
pub mod vfile;
pub mod vfs;

use std::io::ErrorKind;
use std::path::PathBuf;

use log::warn;

use crate::config::{EngineOptions};
use crate::fs::canonicalize;

/// Returns the path to the assets directory.
/// It contains mods and externalized subdirectories.
pub fn get_assets_dir() -> PathBuf {
    let extra_data_dir_env = option_env!("EXTRA_DATA_DIR").unwrap_or(".");
    let extra_data_dir = std::env::current_exe()
        .and_then(|exe| {
            exe.parent()
                .map(|p| p.join(extra_data_dir_env))
                .ok_or_else(|| ErrorKind::NotFound.into())
        })
        .and_then(canonicalize);

    match extra_data_dir {
        Ok(current_exe_dir) => current_exe_dir,
        Err(e) => {
            warn!(
                "Defaulting assets dir to {:?} because current exe dir could not determined. Reason: {:?}",
                extra_data_dir_env,
                e
            );
            extra_data_dir_env.into()
        }
    }
}
