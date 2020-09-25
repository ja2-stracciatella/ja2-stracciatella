//! This module implements logging facilities.
//!
//! # FFI
//!
//! [`stracciatella_c_api::c::logger`] contains a C interface for this module.
//!
//! [`stracciatella_c_api::c::logger`]: ../../stracciatella_c_api/c/logger/index.html

use std::fs::File;
use std::path::Path;
use std::sync::atomic::{AtomicUsize, Ordering};

use log::{
    logger, set_boxed_logger, set_max_level, warn, Level, LevelFilter, Log, Metadata,
    MetadataBuilder, Record,
};
use simplelog::{
    CombinedLogger, Config, SharedLogger, SimpleLogger, TermLogger, TerminalMode, WriteLogger,
};

static GLOBAL_LOG_LEVEL: AtomicUsize = AtomicUsize::new(LogLevel::Info as usize);

#[derive(Debug, PartialEq, Copy, Clone)]
#[repr(C)]
/// Enum to represent log levels in the application
pub enum LogLevel {
    Error = 0,
    Warn = 1,
    Info = 2,
    Debug = 3,
    Trace = 4,
}

impl From<LogLevel> for Level {
    fn from(other: LogLevel) -> Level {
        match other {
            LogLevel::Debug => Level::Debug,
            LogLevel::Error => Level::Error,
            LogLevel::Info => Level::Info,
            LogLevel::Trace => Level::Trace,
            LogLevel::Warn => Level::Warn,
        }
    }
}

impl From<LogLevel> for usize {
    fn from(other: LogLevel) -> usize {
        other as usize
    }
}

impl From<usize> for LogLevel {
    fn from(other: usize) -> LogLevel {
        match other {
            0 => LogLevel::Error,
            1 => LogLevel::Warn,
            2 => LogLevel::Info,
            3 => LogLevel::Debug,
            4 => LogLevel::Trace,
            _ => panic!("Unexpected log level: {}", other),
        }
    }
}

/// Runtime level filter to filter messages based on a global variable
///
/// Other log levels should be set to max level in order for the filter
/// to work properly
struct RuntimeLevelFilter {
    logger: Box<dyn Log>,
}

impl RuntimeLevelFilter {
    fn init(logger: Box<dyn Log>) {
        let filter = RuntimeLevelFilter { logger };

        set_max_level(LevelFilter::max());
        set_boxed_logger(Box::new(filter)).unwrap();
    }

    fn get_global_log_level() -> Level {
        let current_level = GLOBAL_LOG_LEVEL.load(Ordering::Relaxed);
        LogLevel::from(current_level).into()
    }
}

impl Log for RuntimeLevelFilter {
    fn enabled(&self, metadata: &Metadata) -> bool {
        let current_level = Self::get_global_log_level();
        metadata.level() <= current_level
    }

    fn log(&self, record: &Record) {
        if self.enabled(record.metadata()) {
            self.logger.log(record);
        }
    }

    fn flush(&self) {
        self.logger.flush()
    }
}

/// Convenience struct to group logging functionality
pub struct Logger;

impl Logger {
    /// Initializes the logging system
    ///
    /// Needs to be called once at start of the game engine. Any log messages send
    /// before will be discarded.
    pub fn init(log_file: &Path) {
        #[cfg(not(target_os = "android"))]
        {
            let mut config = Config::default();
            config.target = Some(Level::Error);
            config.thread = None;
            config.time_format = Some("%FT%T");
            let logger: Box<dyn SharedLogger>;

            if let Some(termlogger) =
                TermLogger::new(LevelFilter::max(), config, TerminalMode::Mixed)
            {
                logger = termlogger;
            } else {
                logger = SimpleLogger::new(LevelFilter::max(), config); // no colors
            }

            match File::create(&log_file) {
                Ok(f) => RuntimeLevelFilter::init(CombinedLogger::new(vec![
                    logger,
                    WriteLogger::new(LevelFilter::max(), config, f),
                ])),
                Err(err) => {
                    RuntimeLevelFilter::init(CombinedLogger::new(vec![logger]));
                    warn!("Failed to log to {:?}: {}", &log_file, err);
                }
            }
        }
        #[cfg(target_os = "android")]
        {
            let config = android_logger::Config::default()
                .with_min_level(Level::Trace)
                .with_tag("JA2");
            RuntimeLevelFilter::init(Box::new(android_logger::AndroidLogger::new(config)));
        }
    }

    /// Sets the global log level to a specific value
    pub fn set_level(level: LogLevel) {
        GLOBAL_LOG_LEVEL.store(level.into(), Ordering::Relaxed);
    }

    /// Logs message with specific metadata
    ///
    /// Can be used e.g. in C++ or scripting
    pub fn log_with_custom_metadata(level: LogLevel, message: &str, target: &str) {
        let level = level.into();
        let logger = logger();
        let metadata = MetadataBuilder::new().level(level).target(target).build();

        if logger.enabled(&metadata) {
            logger.log(
                &Record::builder()
                    .metadata(metadata)
                    .args(format_args!("{}", message))
                    .build(),
            );
        }
    }
}
