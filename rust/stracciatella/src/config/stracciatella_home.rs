use crate::fs::resolve_existing_components;
use std::path::{Path, PathBuf};

#[cfg(not(windows))]
const STRACCIATELLA_HOME_DIR_NAME: &str = ".ja2";
#[cfg(windows)]
const STRACCIATELLA_HOME_DIR_NAME: &str = "JA2";

#[cfg(not(windows))]
/// Find ja2 stracciatella configuration directory inside the user's home directory or the android app dir
pub fn find_stracciatella_home() -> Result<PathBuf, String> {
    #[cfg(not(target_os = "android"))]
	let base: Option<PathBuf>;
	if Path::new(&format!("{}/{}", std::env::var("HOME").unwrap(), STRACCIATELLA_HOME_DIR_NAME)).is_dir() {
		base = dirs::home_dir();
	} else {
		base = dirs::config_dir();
	};

    #[cfg(target_os = "android")]
    let base = match crate::android::get_android_app_dir() {
        Ok(v) => Some(v),
        Err(e) => {
            log::error!("JNI Error: {}", e);
            None
        }
    };

    match base {
	Some(mut path) => {
            path.push(STRACCIATELLA_HOME_DIR_NAME);
            Ok(resolve_existing_components(&path, None, true))
        }
        None => Err(String::from("Could not find home directory")),
    }
}

#[cfg(windows)]
/// Find ja2 stracciatella configuration directory
/// First try the new one inside the appdata directory. If that does not exist and the old one does, return the old one. Otherwise return the new one.
pub fn find_stracciatella_home() -> Result<PathBuf, String> {
    use std::borrow::Cow;

    let get_dir_info = |p: PathBuf| {
        let p = resolve_existing_components(&p.join(STRACCIATELLA_HOME_DIR_NAME), None, true);
        let exists = p.exists();
        (p, exists)
    };
    let old = dirs::document_dir().map(get_dir_info);
    let new = dirs::config_dir().map(get_dir_info);

    if let Some((new, true)) = new {
        // New one already exists, use this one
        return Ok(new);
    }
    if let Some((old, true)) = old {
        // Old one exists, warn and use this one
        log::warn!(
            "The old configuration directory `{}` is deprecated. Please move your data to the new one: `{}`.",
            old.to_string_lossy(),
            new.as_ref()
                .map(|(p, _)| p.to_string_lossy())
                .unwrap_or_else(|| Cow::Borrowed("<failed to find new directory>")),
        );
        return Ok(old);
    }
    if let Some((new, _)) = new {
        // None of them exists, use new one
        return Ok(new);
    }
    Err(String::from("Could not find home directory"))
}

#[cfg(test)]
mod tests {
    #[test]
    #[cfg(all(not(windows), not(target_os = "android")))]
    fn find_stracciatella_home_should_find_the_correct_stracciatella_home_path_on_unixlike() {
        use std::path::Path;
        use crate::config::find_stracciatella_home;

        let stracciatella_home = String::from(find_stracciatella_home().unwrap().to_str().unwrap());
	let mut expected_vec: Vec<String> = Vec::new();
	expected_vec.push(format!("{}/.ja2", std::env::var("HOME").unwrap()));
	expected_vec.push(format!("{}/.ja2", std::env::var("XDG_CONFIG_HOME").unwrap()));
	assert!(expected_vec.contains(&stracciatella_home));
	
    }

    #[test]
    #[cfg(windows)]
    fn find_stracciatella_home_should_find_the_correct_stracciatella_home_path_on_windows() {
        use crate::config::find_stracciatella_home;
        use regex::Regex;

        let stracciatella_home = find_stracciatella_home().unwrap();

        let result = stracciatella_home.to_str().unwrap();
        let regex = Regex::new(r"^[A-Z]:\\(.*)+\\JA2").unwrap();
        assert!(
            regex.is_match(result),
            "{} is not a valid home dir for windows",
            result
        );
    }
}
