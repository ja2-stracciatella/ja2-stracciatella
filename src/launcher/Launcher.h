#ifndef JA2_LAUNCHER_H_H
#define JA2_LAUNCHER_H_H

#include "StracciatellaLauncher.h"

class Launcher : public StracciatellaLauncher {
public:
    void show();
    int writeIniFile();
    int readFromIniOrDefaults();
private:
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
