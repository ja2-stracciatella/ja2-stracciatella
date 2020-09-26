#[cfg(target_os = "android")]
use jni::{errors::Error as JNIError, JNIEnv};
use std::path::PathBuf;

/// Find ja2 stracciatella configuration directory inside the user's home directory
#[cfg(not(target_os = "android"))]
pub fn find_stracciatella_home() -> Result<PathBuf, String> {
    use crate::fs::resolve_existing_components;
    use dirs;

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

/// Find ja2 stracciatella configuration directory for android
/// This is a separate function as it needs the jnienv argument
#[cfg(target_os = "android")]
pub fn find_stracciatella_home(jni_env: JNIEnv<'_>) -> Result<PathBuf, JNIError> {
    let current_activity_thread = jni_env
        .call_static_method(
            "android/app/ActivityThread",
            "currentActivityThread",
            "()Landroid/app/ActivityThread;",
            &[],
        )?
        .l()?;
    let application = jni_env
        .call_method(
            current_activity_thread,
            "getApplication",
            "()Landroid/app/Application;",
            &[],
        )?
        .l()?;
    let context = jni_env
        .call_method(
            application,
            "getApplicationContext",
            "()Landroid/content/Context;",
            &[],
        )?
        .l()?;
    let files_dir = jni_env
        .call_method(context, "getFilesDir", "()Ljava/io/File;", &[])?
        .l()?;
    let path = jni_env.get_string(
        jni_env
            .call_method(files_dir, "getAbsolutePath", "()Ljava/lang/String;", &[])?
            .l()?
            .into(),
    )?;
    let path_string: String = path.into();

    Ok(PathBuf::from(&path_string))
}
