//! This module contains the C interface for [`stracciatella::logger`].
//!
//! [`stracciatella::logger`]: ../../../stracciatella/logger/index.html

pub use stracciatella::logger::LogLevel;
use stracciatella::logger::Logger;

use crate::c::common::*;

/// Initializes the logger
#[no_mangle]
pub extern "C" fn Logger_initialize(log_file_name: *const c_char) {
    let log_file = str_from_c_str_or_panic(unsafe_c_str(log_file_name));
    Logger::init(&log_file)
}

/// Returns the path to the log file specified by `log_file_name`
#[no_mangle]
pub extern "C" fn Logger_getFilePath(log_file_name: *const c_char) -> *mut c_char {
    let log_file = str_from_c_str_or_panic(unsafe_c_str(log_file_name));
    let file_name = c_string_from_path_or_panic(&Logger::get_log_file_path(&log_file));
    file_name.into_raw()
}

/// Set log level
#[no_mangle]
pub extern "C" fn Logger_setLevel(level: LogLevel) {
    Logger::set_level(level)
}

/// Log with custom metadata
#[no_mangle]
pub extern "C" fn Logger_log(level: LogLevel, message: *const c_char, target: *const c_char) {
    let message = str_from_c_str_or_panic(unsafe_c_str(message));
    let target = str_from_c_str_or_panic(unsafe_c_str(target));

    Logger::log_with_custom_metadata(level, message, &target);
}
