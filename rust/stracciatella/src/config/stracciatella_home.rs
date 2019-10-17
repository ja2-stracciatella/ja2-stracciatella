use crate::fs::resolve_existing_components;
use dirs;
use std::path::PathBuf;

/// Find ja2 stracciatella configuration directory inside the user's home directory
pub fn find_stracciatella_home() -> Result<PathBuf, String> {
    #[cfg(not(windows))]
    let base = dirs::home_dir();
    #[cfg(windows)]
    let base = dirs::document_dir();
    #[cfg(not(windows))]
    let dir = ".ja2";
    #[cfg(windows)]
    let dir = "JA2";

    match base {
        Some(mut path) => {
            path.push(dir);
            Ok(resolve_existing_components(&path, None, true))
        }
        None => Err(String::from("Could not find home directory")),
    }
}
