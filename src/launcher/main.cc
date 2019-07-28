#include <string>
#include <FL/Fl.H>

#include <Launcher.h>
#include "RustInterface.h"

int main(int argc, char* argv[]) {
	Logger_Init("stracciatella-launcher.log");

	Launcher launcher(argc, argv);
	launcher.loadJa2Json();
	launcher.show();
	return Fl::run();
}
