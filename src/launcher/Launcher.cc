#include <string>
#include "FL/Fl_Native_File_Chooser.H"
#include <boost/foreach.hpp>
#include <MicroIni/MicroIni.hpp>
#include "slog/slog.h"

#include "FileMan.h"
#include "Launcher.h"

#define LAUNCHER_TOPIC "Launcher"

#define GAME_SECTION ""
#define LAUNCHER_SECTION "launcher"
#define DATA_DIR_KEY "data_dir"
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

std::string getJa2Directory() {
    return FileMan::findConfigFile(FileMan::findConfigFolderAndSwitchIntoIt());
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
    const std::string configPath = getJa2Directory();
    MicroIni::File file;

    file[GAME_SECTION][DATA_DIR_KEY] = dataDirectoryInput->value();

    file[LAUNCHER_SECTION][GAME_VERSION_KEY] = gameVersionInput->value();
    if (customResolutionButton->value()) {
        char res[80];
        sprintf(res, "%dx%d", (int)customResolutionXInput->value(), (int)customResolutionYInput->value());
        file[LAUNCHER_SECTION][RESOLUTION_KEY] = res;
    } else {
        file[LAUNCHER_SECTION][RESOLUTION_KEY] = predefinedResolutionInput->value();
    }
    file[LAUNCHER_SECTION][FULLSCREEN_KEY] = fullscreenCheckbox->value() == 1 ? "true" : "false";
    file[LAUNCHER_SECTION][PLAY_SOUNDS_KEY] = playSoundsCheckbox->value() == 1 ? "true" : "false";

    if(!file.save(configPath)) {
        SLOGW(LAUNCHER_TOPIC, "Failed writing to file\" %s", configPath.c_str());
        return 1;
    }
    SLOGD(LAUNCHER_TOPIC, "Succeeded writing to file %s", configPath.c_str());

    return 0;
}

int Launcher::readFromIniOrDefaults() {
    const std::string configPath = getJa2Directory();
    MicroIni::File file;
    bool failedToRead = false;

    if(!file.load(configPath)) {
        failedToRead = true;
        SLOGW(LAUNCHER_TOPIC, "Failed reading from file %s", configPath.c_str());
    } else {
        SLOGD(LAUNCHER_TOPIC, "Succeeded reading from file %s", configPath.c_str());
    }


    if (!failedToRead && file[GAME_SECTION].has(DATA_DIR_KEY)) {
        dataDirectoryInput->value(file[GAME_SECTION][DATA_DIR_KEY].c_str());
    } else {
        dataDirectoryInput->value("/some/place/where/the/data/is");
    }
    if (!failedToRead && file[LAUNCHER_SECTION].has(GAME_VERSION_KEY)) {
        gameVersionInput->value(file[LAUNCHER_SECTION][GAME_VERSION_KEY].c_str());
    } else {
        gameVersionInput->value("ENGLISH");
    }
    if (!failedToRead && file[LAUNCHER_SECTION].has(RESOLUTION_KEY)) {
        const char* resolutionString = file[LAUNCHER_SECTION][RESOLUTION_KEY].c_str();
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
    } else {
        enablePredefinedResolutions();
        predefinedResolutionInput->value(predefinedResolutions[0]);
    }
    if (!failedToRead && file[LAUNCHER_SECTION].has(FULLSCREEN_KEY)) {
        fullscreenCheckbox->value(file[LAUNCHER_SECTION][FULLSCREEN_KEY] == "true" ? 1 : 0);
    } else {
        fullscreenCheckbox->value(1);
    }
    if (!failedToRead && file[LAUNCHER_SECTION].has(PLAY_SOUNDS_KEY)) {
        playSoundsCheckbox->value(file[LAUNCHER_SECTION][PLAY_SOUNDS_KEY] == "true" ? 1 : 0);
    } else {
        playSoundsCheckbox->value(1);
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


