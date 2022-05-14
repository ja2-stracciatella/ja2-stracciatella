//! This module and it's submodules contains threadsafe C bindings to std::process.

use libc::c_char;
use std::path::PathBuf;
use std::process::{Child, Command, Output};
use std::sync::{Arc, Mutex};

use crate::c::common::*;
use crate::c::vec::*;

pub enum SubProcessState {
    /// The subprocess is still running
    Running(Option<Child>),
    /// The subprocess has finished with status code
    Finished(Arc<Output>),
    /// Communication with the process has failed
    Error(Arc<std::io::Error>),
}

impl SubProcessState {
    /// Run subprocess with arguments
    pub fn new(cmd: std::path::PathBuf, args: Vec<PathBuf>) -> std::io::Result<Self> {
        let cmd = Command::new(cmd).args(args).spawn()?;
        Ok(cmd.into())
    }

    /// Check if process finished or failed
    pub fn is_done(&self) -> bool {
        matches!(self, Self::Finished(_) | Self::Error(_))
    }

    /// Check if process has finished and update state
    pub fn process(&mut self) {
        if let Self::Running(ref mut c) = self {
            let mut c = std::mem::take(c).expect("SubProcess::Running child should always be some");
            *self = match c.try_wait() {
                Ok(Some(_)) => match c.wait_with_output() {
                    Ok(o) => o.into(),
                    Err(e) => Self::Error(Arc::new(e)),
                },
                Ok(None) => Self::Running(Some(c)),
                Err(e) => Self::Error(Arc::new(e)),
            }
        }
    }

    /// Get subprocess output
    pub fn output(&self) -> Result<Option<Arc<Output>>, Arc<std::io::Error>> {
        match self {
            Self::Finished(o) => Ok(Some(o.clone())),
            Self::Error(e) => Err(e.clone()),
            Self::Running(_) => Ok(None),
        }
    }
}

impl From<Child> for SubProcessState {
    fn from(c: Child) -> Self {
        Self::Running(Some(c))
    }
}

impl From<Output> for SubProcessState {
    fn from(o: Output) -> Self {
        Self::Finished(Arc::new(o))
    }
}

pub struct SubProcess {
    inner: Arc<Mutex<SubProcessState>>,
}

impl SubProcess {
    /// Run subprocess with arguments
    pub fn new(cmd: std::path::PathBuf, args: Vec<PathBuf>) -> std::io::Result<Self> {
        let cmd = Command::new(cmd).args(args).spawn()?;
        Ok(Self {
            inner: Arc::new(Mutex::new(cmd.into())),
        })
    }

    /// Check if process finished or failed
    pub fn is_done(&self) -> bool {
        let inner = self.inner.lock().expect("subprocess mutex poisoned");
        inner.is_done()
    }

    /// Check if process has finished and update state
    pub fn process(&mut self) {
        let mut inner = self.inner.lock().expect("subprocess mutex poisoned");
        inner.process()
    }

    /// Get subprocess output.
    pub fn output(&self) -> Result<Option<Arc<Output>>, Arc<std::io::Error>> {
        let inner = self.inner.lock().expect("subprocess mutex poisoned");
        inner.output()
    }
}

/// Starts a subprocess and collects stdin/stderr in the process
///
/// Returns null on error
#[no_mangle]
pub extern "C" fn Subprocess_new(program: *const c_char, args: *mut VecCString) -> *mut SubProcess {
    forget_rust_error();

    let program = path_buf_from_c_str_or_panic(unsafe_c_str(program));
    let args: Vec<_> = unsafe_ref(args)
        .inner
        .iter()
        .map(|x| path_buf_from_c_str_or_panic(x))
        .collect();
    let cmd = SubProcess::new(program, args);

    match cmd {
        Ok(cmd) => into_ptr(cmd),
        Err(e) => {
            remember_rust_error(format!("{}", e));
            std::ptr::null_mut()
        }
    }
}

/// Checks whether the subprocess is done running
///
/// Also considers the subprocess as finished when a communication error occurs
#[no_mangle]
pub extern "C" fn Subprocess_isDone(ptr: *mut SubProcess) -> bool {
    let ptr = unsafe_mut(ptr);
    ptr.is_done()
}

/// Maintains internal subprocess state. Will mark it as done when subprocess has finished
#[no_mangle]
pub extern "C" fn Subprocess_process(ptr: *mut SubProcess) {
    let ptr = unsafe_mut(ptr);
    ptr.process()
}

/// Gets the subprocesses exit code. Throws an error if the process has not finished yet.
///
/// Also throws an error when a communication error with the subprocess occured
///
/// # Safety
///
/// We use `libc::strsignal` to create a signal string for the error message. If the result is not valid utf8 the function will crash.
#[no_mangle]
pub unsafe extern "C" fn Subprocess_getExitCode(ptr: *mut SubProcess) -> i32 {
    forget_rust_error();

    let ptr = unsafe_mut(ptr);
    match ptr.output() {
        Ok(Some(output)) => {
            let exit_code = output.status.code();
            if let Some(exit_code) = exit_code {
                exit_code
            } else {
                let mut error = "Subprocess terminated by a signal.".to_owned();
                #[cfg(target_family = "unix")]
                {
                    // Include signal details on unix systems
                    use std::os::unix::process::ExitStatusExt;

                    if let Some(signal) = output.status.signal() {
                        let signal_ptr = libc::strsignal(signal);
                        if !signal_ptr.is_null() {
                            let signal_str = str_from_c_str_or_panic(unsafe_c_str(signal_ptr));
                            error = format!("Subprocess terminated by signal: {}", signal_str);
                        }
                    }
                }
                remember_rust_error(error);
                i32::MIN
            }
        }
        Ok(None) => {
            remember_rust_error("Subprocess has not finished yet.");
            i32::MIN
        }
        Err(e) => {
            remember_rust_error(format!("Error communicating with subprocess: {}", e));
            i32::MIN
        }
    }
}

/// Destroys the SubProcess instance.
/// coverity[+free : arg-0]
#[no_mangle]
pub extern "C" fn SubProcess_destroy(ptr: *mut SubProcess) {
    let _drop_me = from_ptr(ptr);
}
