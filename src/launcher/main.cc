#include <string>
#include <FL/Fl.H>
#include <slog/slog.h>

#include <FileMan.h>
#include <Launcher.h>
#include "RustInterface.h"

const std::string getJa2Executable(char* argv[]) {
    std::string exeFolder = FileMan::getParentPath(argv[0], true);
    #ifdef _WIN32
        return FileMan::joinPaths(exeFolder, "ja2.exe");
    #else
        return FileMan::joinPaths(exeFolder, "ja2");
    #endif
}

int main(int argc, char* argv[]) {
    SLOG_Init(SLOG_STDERR, "stracciatella-launcher.log");
    SLOG_SetLevel(SLOG_WARNING, SLOG_WARNING);

    engine_options_t* params = create_engine_options(argv, argc);
    if (params == NULL) {
      return EXIT_FAILURE;
    }

    Launcher launcher(getJa2Executable(argv), params);

    launcher.show();
    return Fl::run();
}
