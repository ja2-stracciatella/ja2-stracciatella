use std::path::{Path, PathBuf};
use std::sync::RwLock;

use jni::{
    JNIEnv,
    errors::{Error, Result},
    objects::{JObject, JString, JValue},
};
use lazy_static::lazy_static;
use ndk::asset::AssetManager;

struct GlobalJNIEnv(pub *mut jni::sys::JNIEnv);

unsafe impl Sync for GlobalJNIEnv {}

unsafe impl Send for GlobalJNIEnv {}

lazy_static! {
    /// This is the global jni env
    /// When the engine is powered up, we expect this to be set once by calling set_global_jni_env.
    ///
    /// We expect the passed pointer to be valid for all subsequent calls that use jni.
    static ref GLOBAL_JNI_ENV: RwLock<GlobalJNIEnv> = {
        RwLock::new(GlobalJNIEnv(std::ptr::null_mut()))
    };
}

/// Set the global jni env for the stracciatella lib.
///
/// This should be called with the SDL JNI env
pub fn set_global_jni_env(jni_env: *mut jni::sys::JNIEnv) -> Result<()> {
    if jni_env.is_null() {
        return Err(Error::NullPtr("jni_env").into());
    }
    let mut global_jni_env = GLOBAL_JNI_ENV.try_write().map_err(|_| Error::TryLock)?;
    global_jni_env.0 = jni_env;
    Ok(())
}

/// Gets the rust version of the current global jni env
pub fn get_global_jni_env() -> Result<jni::JNIEnv<'static>> {
    let global_jni_env = GLOBAL_JNI_ENV.try_read().map_err(|_| Error::TryLock)?;
    unsafe { JNIEnv::from_raw(global_jni_env.0) }
}

/// Arbitraily chosen value for a small JNI frame
const SMALL_JNI_FRAME_SIZE: i32 = 16;

/// Get current application context form JNI env
pub fn get_application_context() -> Result<JObject<'static>> {
    let jni_env = get_global_jni_env()?;
    jni_env.with_local_frame(SMALL_JNI_FRAME_SIZE, || {
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
        jni_env
            .call_method(
                application,
                "getApplicationContext",
                "()Landroid/content/Context;",
                &[],
            )?
            .l()
    })
}

fn get_dir_from_application_context(method_name: &'static str) -> Result<PathBuf> {
    let jni_env = crate::android::get_global_jni_env()?;
    let path_obj = jni_env.auto_local(jni_env.with_local_frame(SMALL_JNI_FRAME_SIZE, || {
        let context = crate::android::get_application_context()?;
        let files_dir = jni_env
            .call_method(context, method_name, "()Ljava/io/File;", &[])?
            .l()?;
        jni_env
            .call_method(files_dir, "getAbsolutePath", "()Ljava/lang/String;", &[])?
            .l()
    })?);
    let path = jni_env.get_string(path_obj.as_obj().into())?;
    let path_string: String = path.into();

    Ok(PathBuf::from(&path_string))
}

/// Find ja2 stracciatella configuration directory for android
pub fn get_android_app_dir() -> Result<PathBuf> {
    get_dir_from_application_context("getFilesDir")
}

/// Find cache directory for android
pub fn get_android_cache_dir() -> Result<PathBuf> {
    get_dir_from_application_context("getCacheDir")
}

/// Get asset manager from JNI env
pub fn get_asset_manager() -> Result<AssetManager> {
    let jni_env = get_global_jni_env()?;
    // Not using AutoLocal for asset_manager, because we dont want it to be cleaned up
    let asset_manager = jni_env.with_local_frame(SMALL_JNI_FRAME_SIZE, || {
        let context = get_application_context()?;
        jni_env
            .call_method(
                context,
                "getAssets",
                "()Landroid/content/res/AssetManager;",
                &[],
            )?
            .l()
    })?;
    Ok(unsafe {
        // This is the cast of death
        let ptr = ndk_sys::AAssetManager_fromJava(
            (jni_env.get_native_interface() as *const jni_sys::JNINativeInterface_)
                as *mut *const jni_sys::JNINativeInterface_,
            asset_manager.into_inner() as *mut jni_sys::_jobject,
        );
        AssetManager::from_ptr(std::ptr::NonNull::new(ptr).unwrap())
    })
}

/// List directory in Assets (including directories)
pub fn list_asset_dir(dir: &Path) -> Result<Vec<PathBuf>> {
    let jni_env = get_global_jni_env()?;
    let mut results: Vec<PathBuf> = vec![];

    jni_env.auto_local(jni_env.with_local_frame(SMALL_JNI_FRAME_SIZE, || {
        let context = get_application_context()?;
        let asset_manager = jni_env
            .call_method(
                context,
                "getAssets",
                "()Landroid/content/res/AssetManager;",
                &[],
            )?
            .l()?;
        let path = dir
            .to_str()
            .map(String::from)
            .ok_or_else(|| Error::WrongJValueType("string", "list_asset_dir"))?;
        let path_obj = jni_env.new_string(&path)?.into();
        let path = JValue::Object(path_obj);
        let list_contents_obj = jni_env
            .call_method(
                asset_manager,
                "list",
                "(Ljava/lang/String;)[Ljava/lang/String;",
                &[path],
            )?
            .l()?;
        let list_contents = list_contents_obj.into_inner();

        let n_elements = jni_env.get_array_length(list_contents)?;
        for i in 0..n_elements {
            let path = jni_env.auto_local(jni_env.get_object_array_element(list_contents, i)?);
            let l: String = jni_env.get_string(JString::from(path.as_obj()))?.into();
            results.push(PathBuf::from(&l));
        }

        Ok(list_contents_obj)
    })?);

    Ok(results)
}
