//! This module implements logging facilities.
//!
//! # FFI
//!
//! [`stracciatella::c::logger`] contains a C interface for this module.
//!
//! [`stracciatella::c::logger`]: ../c/logger/index.html

use simplelog::*;
use log::{logger, Level, Record, set_max_level};
use std::fs::File;
use std::path::Path;

#[derive(Debug, PartialEq, Copy, Clone)]
#[repr(C)]
// Enum to represent log levels in the application
pub enum LogLevel {
    Error,
    Warn,
    Info,
    Debug,
    Trace
}

impl From<LogLevel> for Level {
    fn from(other: LogLevel) -> Level {
        match other {
            LogLevel::Debug => Level::Debug,
            LogLevel::Error => Level::Error,
            LogLevel::Info => Level::Info,
            LogLevel::Trace => Level::Trace,
            LogLevel::Warn => Level::Warn
        }
    }
}

impl From<LogLevel> for LevelFilter {
    fn from(other: LogLevel) -> LevelFilter {
        match other {
            LogLevel::Debug => LevelFilter::Debug,
            LogLevel::Error => LevelFilter::Error,
            LogLevel::Info => LevelFilter::Info,
            LogLevel::Trace => LevelFilter::Trace,
            LogLevel::Warn => LevelFilter::Warn
        }
    }
}

pub struct Logger;

impl Logger {
    // Initializes the logging system
    //
    // Needs to be called once at start of the game engine before any log messages are sent
    pub fn init(log_file: &Path) {
        CombinedLogger::init(
            vec![
                TermLogger::new(LevelFilter::Trace, Config::default(), TerminalMode::Mixed).unwrap(),
                WriteLogger::new(LevelFilter::Trace, Config::default(), File::create(log_file).unwrap()),
            ]
        ).unwrap();
        Self::set_level(LogLevel::Warn);

    }

    pub fn set_level(level: LogLevel) {
        set_max_level(level.into())
    }

    // Logs message with specific metadata
    //
    // Can be used e.g. in C++ or scripting
    pub fn log_with_custom_metadata(level: LogLevel, message: &str) {
        let level = level.into();
        logger().log(&Record::builder()
            .level(level)
            .args(format_args!("{}", message))
            .build());
    }
}