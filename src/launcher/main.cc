#include <string>
#include <FL/Fl.H>
#include <slog/slog.h>

#include <Launcher.h>
#include "RustInterface.h"

int main(int argc, char* argv[]) {
	SLOG_Init(SLOG_STDERR, "stracciatella-launcher.log");
	SLOG_SetLevel(SLOG_WARNING, SLOG_WARNING);

	Launcher launcher(argc, argv);
	launcher.loadJa2Json();
	launcher.show();
	return Fl::run();
}
