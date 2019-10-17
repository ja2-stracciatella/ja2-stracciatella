//! This module contains code to interact with the filesystem.

use dunce;

/// Returns the canonical, absolute form of a path with all intermediate
/// components normalized and symbolic links resolved.
///
/// On windows, UNC paths are converted to normal paths when possible.
pub use dunce::canonicalize;
