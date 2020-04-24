#include <Launcher.h>
#include "RustInterface.h"

#include <FL/Fl.H>


int main(int argc, char* argv[]) {
	Logger_initialize("ja2-launcher.log");

	Launcher launcher(argc, argv);
	launcher.loadJa2Json();
	launcher.show();
	return Fl::run();
}
