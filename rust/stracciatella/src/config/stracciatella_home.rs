use std::path::PathBuf;

/// Find ja2 stracciatella configuration directory inside the user's home directory
pub fn find_stracciatella_home() -> Result<PathBuf, String> {
    use crate::fs::resolve_existing_components;

    #[cfg(all(not(windows), not(target_os = "android")))]
    let base = dirs::home_dir();
    #[cfg(target_os = "android")]
    let base = match crate::android::get_android_app_dir() {
        Ok(v) => Some(v),
        Err(e) => {
            log::error!("JNI Error: {}", e);
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

#[cfg(test)]
mod tests {
    use std::path::Path;

    use crate::*;

    #[test]
    #[cfg(all(not(windows), not(target_os = "android")))]
    fn find_stracciatella_home_should_find_the_correct_stracciatella_home_path_on_unixlike() {
        use crate::config::find_stracciatella_home;

        let mut engine_options = EngineOptions::default();
        engine_options.stracciatella_home = find_stracciatella_home().unwrap();
        let expected = format!("{}/.ja2", std::env::var("HOME").unwrap());

        assert_eq!(engine_options.stracciatella_home, Path::new(&expected));
    }

    #[test]
    #[cfg(windows)]
    fn find_stracciatella_home_should_find_the_correct_stracciatella_home_path_on_windows() {
        use crate::config::find_stracciatella_home;
        use regex::Regex;

        let mut engine_options = EngineOptions::default();
        engine_options.stracciatella_home = find_stracciatella_home().unwrap();

        let result = engine_options.stracciatella_home.to_str().unwrap();
        let regex = Regex::new(r"^[A-Z]:\\(.*)+\\JA2").unwrap();
        assert!(
            regex.is_match(result),
            "{} is not a valid home dir for windows",
            result
        );
    }
}
