#include "Launcher.h"
#include "RustInterface.h"
#include <FL/Fl.H>
#include <string_theory/string>

int main(int argc, char* argv[])
try
{
	Logger_initialize("ja2-launcher.log");

#ifdef _WIN32
	// Ensure quick-edit mode is off, or else it will block execution
	HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
	SetConsoleMode(hInput, ENABLE_EXTENDED_FLAGS);
#endif

	Launcher launcher(argc, argv);
	launcher.loadJa2Json();
	launcher.show();
	return Fl::run();
}
catch (...)
{
	// If you ever see return code 27, try to set a breakpoint here
	return 27;
}
