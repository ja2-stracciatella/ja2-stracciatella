#include <string>
#include <FL/Fl.H>
#include <slog/slog.h>

#include <Launcher.h>
#include "RustInterface.h"

int main(int argc, char* argv[]) {
    SLOG_Init(SLOG_STDERR, "stracciatella-launcher.log");
    SLOG_SetLevel(SLOG_WARNING, SLOG_WARNING);

    char* rustExePath = find_ja2_executable(argv[0]);
    std::string exePath = std::string(rustExePath);
    free_rust_string(rustExePath);

    engine_options_t* params = create_engine_options(argv, argc);

    if (params == NULL) {
      return EXIT_FAILURE;
    }
    if (should_show_help(params)) {
      return EXIT_SUCCESS;
    }

    Launcher launcher(exePath, params);

    launcher.show();
    return Fl::run();
}
