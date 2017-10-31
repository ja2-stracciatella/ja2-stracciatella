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

int Launcher::writeIniFile() {
    std::ofstream ofs(configPath.c_str());

    if (ofs) {
        rapidjson::OStreamWrapper osw(ofs);

        rapidjson::Document document;
        document.SetObject();

        rapidjson::Document::AllocatorType &allocator = document.GetAllocator();
        rapidjson::Value val;

        val.SetString(rapidjson::StringRef(helpString.c_str()));
        document.AddMember(HELP_KEY, val, allocator);

        val.SetString(rapidjson::StringRef(dataDirectoryInput->value()));
        document.AddMember(DATA_DIR_KEY, val, allocator);

        rapidjson::Value launcher_section(rapidjson::kObjectType);
        val.SetString(rapidjson::StringRef(gameVersionInput->value()));
        launcher_section.AddMember(GAME_VERSION_KEY, val, allocator);

        char res[80];
        if (customResolutionButton->value()) {
            sprintf(res, "%dx%d", (int)customResolutionXInput->value(), (int)customResolutionYInput->value());
            val.SetString(rapidjson::StringRef(res));
        } else {
            val.SetString(rapidjson::StringRef(predefinedResolutionInput->value()));
        }
        launcher_section.AddMember(RESOLUTION_KEY, val, allocator);

        val.SetBool(fullscreenCheckbox->value());
        launcher_section.AddMember(FULLSCREEN_KEY, val, allocator);

        val.SetBool(playSoundsCheckbox->value());
        launcher_section.AddMember(PLAY_SOUNDS_KEY, val, allocator);




        document.AddMember(LAUNCHER_SECTION, launcher_section, allocator);


        rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
        document.Accept(writer);

        ofs.close();

        if (ofs) {
            SLOGD(LAUNCHER_TOPIC, "Succeeded writing to file %s", configPath.c_str());
            return 0;
        }
    }
    SLOGW(LAUNCHER_TOPIC, "Failed writing to file\" %s", configPath.c_str());
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

    cmd += std::string(" -resversion ") + std::string(gameVersionInput->value());

    cmd += std::string(" -res ");
    if (customResolutionButton->value()) {
        char res[80];
        sprintf(res, "%dx%d", (int)customResolutionXInput->value(), (int)customResolutionYInput->value());
        cmd += std::string(res);
    } else {
        cmd += std::string(predefinedResolutionInput->value());
    }
    if (!playSoundsCheckbox->value()) {
        cmd += std::string(" -nosound");
    }
    if (fullscreenCheckbox->value()) {
        cmd += std::string(" -fullscreen");
    }
    if (asEditor) {
        cmd += std::string(" -editor");
    }

    system(cmd.c_str());
}

void Launcher::startGame(Fl_Widget* btn, void* userdata) {
    Launcher* window = static_cast< Launcher* >( userdata );

    window->writeIniFile();
    window->startExecutable(false);
}

void Launcher::startEditor(Fl_Widget* btn, void* userdata) {
    Launcher* window = static_cast< Launcher* >( userdata );

    window->writeIniFile();
    window->startExecutable(true);
}
