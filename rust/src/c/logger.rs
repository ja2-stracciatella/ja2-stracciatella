//! This module contains the C interface for [`stracciatella::logger`].
//!
//! [`stracciatella::logger`]: ../../logger/index.html

use crate::c::common::*;
use crate::logger::Logger;

pub use crate::logger::LogLevel;

/// Initializes the logger
#[no_mangle]
pub extern "C" fn Logger_Init(log_file: *const c_char) {
    let log_file = path_from_c_str_or_panic(unsafe_c_str(log_file));
    Logger::init(log_file)
}

/// Set log level
#[no_mangle]
pub extern "C" fn Logger_SetLevel(level: LogLevel) {
    Logger::set_level(level)
}

/// Log with custom metadata
#[no_mangle]
pub extern "C" fn Logger_LogWithCustomMetadata(
    level: LogLevel,
    message: *const c_char,
    target: *const c_char,
) {
    let message = str_from_c_str_or_panic(unsafe_c_str(message));
    let target = str_from_c_str_or_panic(unsafe_c_str(target));

    Logger::log_with_custom_metadata(level, message, target);
}
