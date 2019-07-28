#include <string>
#include <FL/Fl.H>
#include <slog/slog.h>

#include <Launcher.h>
#include "RustInterface.h"

int main(int argc, char* argv[]) {
	Logger_Init("stracciatella-launcher.log");
	Logger_SetLevel(LogLevel::Warn);

	Launcher launcher(argc, argv);
	launcher.loadJa2Json();
	launcher.show();
	return Fl::run();
}
