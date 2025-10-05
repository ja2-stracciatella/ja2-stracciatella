//! This module implements logging facilities.
//!
//! # FFI
//!
//! [`stracciatella_c_api::c::logger`] contains a C interface for this module.
//!
//! [`stracciatella_c_api::c::logger`]: ../../stracciatella_c_api/c/logger/index.html

use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicUsize, Ordering};

use log::{
    Level, LevelFilter, Log, Metadata, MetadataBuilder, Record, logger, set_boxed_logger,
    set_max_level,
};

static GLOBAL_LOG_LEVEL: AtomicUsize = AtomicUsize::new(LogLevel::Info as usize);

#[derive(Debug, PartialEq, Eq, Copy, Clone)]
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

impl From<Level> for LogLevel {
    fn from(other: Level) -> LogLevel {
        match other {
            Level::Debug => LogLevel::Debug,
            Level::Error => LogLevel::Error,
            Level::Info => LogLevel::Info,
            Level::Trace => LogLevel::Trace,
            Level::Warn => LogLevel::Warn,
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
        if set_boxed_logger(Box::new(filter)).is_err() {
            log::warn!("Error initializing logger: Logger already set");
        }
    }

    fn get_global_log_level() -> Level {
        let current_level = GLOBAL_LOG_LEVEL.load(Ordering::Relaxed);
        LogLevel::from(current_level).into()
    }

    fn set_global_log_level(level: Level) {
        GLOBAL_LOG_LEVEL.store(LogLevel::from(level).into(), Ordering::Relaxed);
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
    /// Gets the path to the current
    pub fn get_log_file_path<P: AsRef<Path>>(log_file_name: P) -> PathBuf {
        #[cfg(not(target_os = "android"))]
        let dir = std::env::temp_dir();
        #[cfg(target_os = "android")]
        let dir = crate::android::get_android_cache_dir().expect("cache dir not found");

        dir.join(log_file_name)
    }

    /// Initializes the logging system
    ///
    /// Needs to be called once at start of the game engine. Any log messages send
    /// before will be discarded.
    pub fn init(log_file_name: &str) {
        use log::warn;
        use simplelog::{
            ColorChoice, CombinedLogger, ConfigBuilder, SharedLogger, TermLogger, TerminalMode,
            ThreadLogMode, WriteLogger,
        };
        use std::fs::File;

        let log_file = Self::get_log_file_path(log_file_name);
        let mut config = ConfigBuilder::default();
        config.set_target_level(LevelFilter::Error);
        config.set_thread_mode(ThreadLogMode::IDs);
        config.set_time_format_rfc3339();
        let config = config.build();
        let logger: Box<dyn SharedLogger> = TermLogger::new(
            LevelFilter::max(),
            config.clone(),
            TerminalMode::Mixed,
            ColorChoice::Auto,
        );

        match File::create(&log_file) {
            Ok(f) => {
                RuntimeLevelFilter::init(CombinedLogger::new(vec![
                    logger,
                    WriteLogger::new(LevelFilter::max(), config, f),
                ]));
                log::info!("Logging to file {:?}", &log_file);
            }
            Err(err) => {
                RuntimeLevelFilter::init(CombinedLogger::new(vec![logger]));
                warn!("Failed to log to {:?}: {}", &log_file, err);
            }
        }
    }

    /// Sets the global log level to a specific value
    pub fn set_level(level: LogLevel) {
        RuntimeLevelFilter::set_global_log_level(level.into())
    }

    /// Gets the global log level
    pub fn get_level() -> LogLevel {
        RuntimeLevelFilter::get_global_log_level().into()
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
