#include <string>
#include "FL/Fl_Native_File_Chooser.H"
#include <boost/foreach.hpp>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/filewritestream.h>
#include "slog/slog.h"
#include "RustInterface.h"

#include "FileMan.h" // for joinPaths
#include "Launcher.h"

#include <iostream>
#include <fstream>

#define LAUNCHER_TOPIC DEBUG_TAG_LAUNCHER

#define GAME_SECTION ""
#define LAUNCHER_SECTION "launcher"
#define DATA_DIR_KEY "data_dir"
#define HELP_KEY "help"
#define GAME_VERSION_KEY "gameVersion"
#define RESOLUTION_KEY "resolution"
#define FULLSCREEN_KEY "fullscreen"
#define PLAY_SOUNDS_KEY "playSounds"

#define RESOLUTION_SEPARATOR "x"

const char* predefinedResolutions[] = {
        "640x480",
        "800x600",
        "1024x768",
        "1280x720",
        "1600x900",
        "1920x1080",
        NULL
};
const char* predefinedVersions[] = {
        "DUTCH",
        "ENGLISH",
        "FRENCH",
        "GERMAN",
        "ITALIAN",
        "POLISH",
        "RUSSIAN",
        "RUSSIAN_GOLD",
        NULL
};



void Launcher::setConfigPath(std::string configPath) {
    this->configPath = FileMan::joinPaths(configPath, "ja2.json");
}

Launcher::Launcher(const std::string exePath, engine_options_t* initialParams) : StracciatellaLauncher() {
    this->exePath = exePath;
    this->initialParams = initialParams;
}

void Launcher::show() {
    browseJa2DirectoryButton->callback((Fl_Callback *) openDataDirectorySelector, (void *) (this));
    predefinedResolutionButton->callback( (Fl_Callback*)selectPredefinedResolution, (void*)(this) );
    customResolutionButton->callback( (Fl_Callback*)selectCustomResolution, (void*)(this) );
    playButton->callback( (Fl_Callback*)startGame, (void*)(this) );
    editorButton->callback( (Fl_Callback*)startEditor, (void*)(this) );

    populateChoices();
    initializeInputsFromDefaults();

    stracciatellaLauncher->show();
}

void Launcher::initializeInputsFromDefaults() {
    char* rustResRootPath = get_vanilla_data_dir(this->initialParams);
    dataDirectoryInput->value(rustResRootPath);
    free_rust_string(rustResRootPath);

    char* rustResVersion = get_resource_version_string(get_resource_version(this->initialParams));
    gameVersionInput->value(rustResVersion);
    free_rust_string(rustResVersion);

    char resolutionString[255];
    sprintf(resolutionString, "%dx%d", get_resolution_x(this->initialParams), get_resolution_y(this->initialParams));

    bool predefinedResolutionFound = false;
    for (int i=0; predefinedResolutions[i] != NULL; i++) {
        if (strcmp(predefinedResolutions[i], resolutionString) == 0) {
            predefinedResolutionFound = true;
        }
    }
    if (predefinedResolutionFound) {
        enablePredefinedResolutions();
        predefinedResolutionInput->value(resolutionString);
    } else {
        enableCustomResolutions();
        char* res = const_cast<char*>(resolutionString);
        const char* x = strtok(res, RESOLUTION_SEPARATOR);
        const char* y = strtok(NULL, RESOLUTION_SEPARATOR);

        customResolutionXInput->value(atoi(x));
        customResolutionYInput->value(atoi(y));
    }

    fullscreenCheckbox->value(should_start_in_fullscreen(this->initialParams) ? 1 : 0);
    playSoundsCheckbox->value(should_start_without_sound(this->initialParams) ? 1 : 0);
}

int Launcher::writeJsonFile() {
    set_start_in_fullscreen(this->initialParams, fullscreenCheckbox->value());
    set_start_without_sound(this->initialParams, !playSoundsCheckbox->value());

    if (customResolutionButton->value()) {
        set_resolution(this->initialParams, (int)customResolutionXInput->value(), (int)customResolutionYInput->value());
    } else {
        std::string res = predefinedResolutionInput->value();
        int split_index = res.find(RESOLUTION_SEPARATOR);
        int x = atoi(res.substr(0, split_index).c_str());
        int y = atoi(res.substr(split_index+1, res.length()).c_str());
        set_resolution(this->initialParams, x, y);
    }

    set_resource_version(this->initialParams, gameVersionInput->value());

    bool success = write_engine_options(this->initialParams);

    if (success) {
        SLOGE(LAUNCHER_TOPIC, "Succeeded writing config file");
        return 0;
    }
    SLOGE(LAUNCHER_TOPIC, "Failed writing config file");
    return 1;
}

void Launcher::populateChoices() {
    for (int i=0; predefinedVersions[i] != NULL; i++) {
        gameVersionInput->add(predefinedVersions[i]);
    }
    for (int i=0; predefinedResolutions[i] != NULL; i++) {
        predefinedResolutionInput->add(predefinedResolutions[i]);
    }
}

void Launcher::enablePredefinedResolutions() {
    predefinedResolutionButton->value(1);
    customResolutionButton->value(0);
    customResolutionXInput->deactivate();
    customResolutionYInput->deactivate();

    predefinedResolutionInput->activate();
}

void Launcher::enableCustomResolutions() {
    customResolutionButton->value(1);
    predefinedResolutionButton->value(0);
    predefinedResolutionInput->deactivate();

    customResolutionXInput->activate();
    customResolutionYInput->activate();
}

void Launcher::openDataDirectorySelector(Fl_Widget *btn, void *userdata) {
    Launcher* window = static_cast< Launcher* >( userdata );
    Fl_Native_File_Chooser fnfc;
    fnfc.title("Select the original Jagged Alliance 2 install directory");
    fnfc.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);

    switch ( fnfc.show() ) {
        case -1: break;  // ERROR
        case  1: break;  // CANCEL
        default: window->dataDirectoryInput->value(fnfc.filename()); break;  // FILE CHOSEN
    }
}

void Launcher::selectPredefinedResolution(Fl_Widget* btn, void* userdata) {
    Launcher* window = static_cast< Launcher* >( userdata );
    window->enablePredefinedResolutions();
}

void Launcher::selectCustomResolution(Fl_Widget* btn, void* userdata) {
    Launcher* window = static_cast< Launcher* >( userdata );
    window->enableCustomResolutions();
}

void Launcher::startExecutable(bool asEditor) {
    std::string cmd(this->exePath);

    if (asEditor) {
        cmd += std::string(" -editor");
    }

    system(cmd.c_str());
}

void Launcher::startGame(Fl_Widget* btn, void* userdata) {
    Launcher* window = static_cast< Launcher* >( userdata );

    window->writeJsonFile();
    window->startExecutable(false);
}

void Launcher::startEditor(Fl_Widget* btn, void* userdata) {
    Launcher* window = static_cast< Launcher* >( userdata );

    window->writeJsonFile();
    window->startExecutable(true);
}
