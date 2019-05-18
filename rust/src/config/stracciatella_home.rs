use dirs;
use std::path::PathBuf;

/// Find ja2 stracciatella configuration directory inside the users home directory
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
            Ok(path)
        },
        None => Err(String::from("Could not find home directory")),
    }
}