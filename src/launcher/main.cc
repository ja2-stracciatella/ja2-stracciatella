#include <string>
#include <FL/Fl.H>
#include <slog/slog.h>

#include <Launcher.h>
#include "RustInterface.h"

void log_rust_message(uint8_t level, uint16_t topic, const char* message) {
	SLOG_LogMessage((SLOGLevel)level, (SLOGTopics)topic, message);
}

int main(int argc, char* argv[]) {
	SLOG_Init(SLOG_STDERR, "stracciatella-launcher.log");
	SLOG_SetLevel(SLOG_INFO, SLOG_INFO);

	setup_rust_slog_logger(log_rust_message);

	char* rustExePath = find_ja2_executable(argv[0]);
	std::string exePath = std::string(rustExePath);
	free_rust_string(rustExePath);

	EngineOptions* params = create_engine_options(argv, argc);

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
