#ifndef JA2_LAUNCHER_H_H
#define JA2_LAUNCHER_H_H

#include "StracciatellaLauncher.h"
#include "RustInterface.h"

class Launcher : public StracciatellaLauncher {
public:
    Launcher(std::string exePath, engine_options_t* initialParams);

    void show();
    int writeIniFile();
    void initializeInputsFromDefaults();
    void setConfigPath(std::string);
private:
    std::string exePath;
    std::string configPath;
    std::string helpString;
    engine_options_t* initialParams;

    void populateChoices();
    void enablePredefinedResolutions();
    void enableCustomResolutions();
    void startExecutable(bool asEditor);
    static void openDataDirectorySelector(Fl_Widget *btn, void *userdata);
    static void selectPredefinedResolution(Fl_Widget* btn, void* userdata);
    static void selectCustomResolution(Fl_Widget* btn, void* userdata);
    static void startGame(Fl_Widget* btn, void* userdata);
    static void startEditor(Fl_Widget* btn, void* userdata);
};

#endif //JA2_LAUNCHER_H_H
