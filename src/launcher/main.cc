#include <string>
#include <FL/Fl.H>
#include <slog/slog.h>

#include <FileMan.h>
#include <Launcher.h>

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

    Launcher launcher(getJa2Executable(argv));
    launcher.show();
    return Fl::run();
}