use std::ffi::CString;
use libc::c_char;
use log::{Log, Record, Level, Metadata};

pub struct SlogLogger(extern "C" fn(u8, u16, *const c_char) -> ());

impl SlogLogger {
    pub fn new(slog_fn: extern "C" fn(u8, u16, *const c_char) -> ()) -> SlogLogger {
        SlogLogger(slog_fn)
    }
}

impl Log for SlogLogger {
    fn enabled(&self, _metadata: &Metadata) -> bool {
        true
    }

    fn log(&self, record: &Record) {
        const RUST_TOPIC: u16 = 63;
        let level = match record.level() {
            Level::Error => 3,
            Level::Warn => 2,
            Level::Info => 1,
            Level::Debug => 0,
            Level::Trace => 0
        };
        let module_path = record.module_path().unwrap_or("unknown module");
        let log_string = format!("[{}] {}", module_path, record.args());
        let log_c_string = CString::new(log_string).unwrap().into_raw();
        self.0(level, RUST_TOPIC, log_c_string);
    }

    fn flush(&self) {}
}