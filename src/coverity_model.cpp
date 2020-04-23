//! Coverity Scan modeling file
//!
//! This file is used to avoid false positives.
//! When changed, it must be uploaded manually in the "Analysis Settings" tab.
//!
//! The following considerations should be kept in mind:
//!  * headers cannot be imported
//!  * builtin types like int/char/void are available, but not wchar_t/NULL/...
//!  * full structs and typedefs are not needed, rudimentary structs and similar types are sufficient
//!  * an uninitialized local pointer means the variable can be null or have some data
//!
//! @see https://scan.coverity.com/projects/ja2-stracciatella-ja2-stracciatella
//! @see https://scan.coverity.com/tune


namespace ST
{
	class string;
}
enum LogLevel { X };


void LogMessage(bool isAssert, LogLevel level, const char* file, const ST::string& str)
{
	if (isAssert)
	{
		// Tell coverity that SLOGA ends execution, which means the tests in Assert/AssertMsg should always succeed.
		// Right now only Debug builds panic, other builds log the message and continue execution.
		__coverity_panic__();
	}
}
