use std::path::PathBuf;

use log::error;

/// Find ja2 stracciatella configuration directory inside the user's home directory
pub fn find_stracciatella_home() -> Result<PathBuf, String> {
    use crate::fs::resolve_existing_components;

    #[cfg(all(not(windows), not(target_os = "android")))]
    let base = dirs::home_dir();
    #[cfg(target_os = "android")]
    let base = match crate::android::get_android_app_dir() {
        Ok(v) => Some(v),
        Err(e) => {
            error!("JNI Error: {}", e);
            None
        }
    };
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
