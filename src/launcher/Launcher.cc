#include <string>
#include "FL/Fl_Native_File_Chooser.H"
#include <boost/foreach.hpp>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/filewritestream.h>
#include "slog/slog.h"

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
        "1920x1080"
};
const char* predefinedVersions[] = {
        "DUTCH",
        "ENGLISH",
        "FRENCH",
        "GERMAN",
        "ITALIAN",
        "POLISH",
        "RUSSIAN",
        "RUSSIAN_GOLD"
};



void Launcher::setConfigPath(std::string configPath) {
    this->configPath = FileMan::joinPaths(configPath, "ja2.json");
}

Launcher::Launcher(const std::string exePath) : StracciatellaLauncher() {
    this->exePath = exePath;
}

void Launcher::show() {
    browseJa2DirectoryButton->callback((Fl_Callback *) openDataDirectorySelector, (void *) (this));
    predefinedResolutionButton->callback( (Fl_Callback*)selectPredefinedResolution, (void*)(this) );
    customResolutionButton->callback( (Fl_Callback*)selectCustomResolution, (void*)(this) );
    playButton->callback( (Fl_Callback*)startGame, (void*)(this) );
    editorButton->callback( (Fl_Callback*)startEditor, (void*)(this) );

    populateChoices();
    readFromIniOrDefaults();

    stracciatellaLauncher->show();
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

int Launcher::readFromIniOrDefaults() {
    std::ifstream configs(configPath.c_str());
    rapidjson::IStreamWrapper configisw(configs);


    rapidjson::Document document;
    document.ParseStream(configisw);

    bool failedToRead = document.Size() < 1;

    if(failedToRead) {
        SLOGW(LAUNCHER_TOPIC, "Failed reading from file %s", configPath.c_str());
    } else {
        SLOGD(LAUNCHER_TOPIC, "Succeeded reading from file %s", configPath.c_str());
    }

    // default values:
    dataDirectoryInput->value("/some/place/where/the/data/is");
    gameVersionInput->value("ENGLISH");
    fullscreenCheckbox->value(1);
    playSoundsCheckbox->value(1);
    bool use_default_resolutions = true;


    if (document.HasMember(DATA_DIR_KEY) && document[DATA_DIR_KEY].IsString()) {
        dataDirectoryInput->value(document[DATA_DIR_KEY].GetString());
    }

    if (document.HasMember(HELP_KEY) && document[HELP_KEY].IsString()) {
        helpString = document[HELP_KEY].GetString();
    } else {
        helpString = "Put the directory to your original ja2 installation into the line below";
    }

    if (document.HasMember(LAUNCHER_SECTION) && document[LAUNCHER_SECTION].IsObject()) {
        const rapidjson::Value& launcher_section = document[LAUNCHER_SECTION];
        if (launcher_section.HasMember(GAME_VERSION_KEY) && launcher_section[GAME_VERSION_KEY].IsString()) {
            gameVersionInput->value(launcher_section[GAME_VERSION_KEY].GetString());
        }
        if (launcher_section.HasMember(FULLSCREEN_KEY) && launcher_section[FULLSCREEN_KEY].IsBool()) {
            fullscreenCheckbox->value(launcher_section[FULLSCREEN_KEY].GetBool());
        }
        if (launcher_section.HasMember(PLAY_SOUNDS_KEY) && launcher_section[PLAY_SOUNDS_KEY].IsBool()) {
            playSoundsCheckbox->value(launcher_section[PLAY_SOUNDS_KEY].GetBool());
        }

        if (launcher_section.HasMember(RESOLUTION_KEY) && launcher_section[RESOLUTION_KEY].IsString()) {
            use_default_resolutions = false;

            const char* resolutionString = launcher_section[RESOLUTION_KEY].GetString();
            bool predefinedResolutionFound = false;

            BOOST_FOREACH(const char* str, predefinedResolutions)
            {
                if (strcmp(str, resolutionString) == 0) {
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
        }
    }

    if (use_default_resolutions) {
        enablePredefinedResolutions();
        predefinedResolutionInput->value(predefinedResolutions[0]);
    }

    return failedToRead ? 1 : 0;
}

void Launcher::populateChoices() {
    BOOST_FOREACH(const char* str, predefinedVersions)
    {
        gameVersionInput->add(str);
    }
    BOOST_FOREACH(const char* str, predefinedResolutions)
    {
        predefinedResolutionInput->add(str);
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


