#include <Launcher.h>
#include "RustInterface.h"

#include <FL/Fl.H>
#include <filesystem>


int main(int argc, char* argv[]) 
{
	auto logPath = std::filesystem::temp_directory_path().append("ja2-launcher.log");
	Logger_initialize(logPath.c_str());

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
