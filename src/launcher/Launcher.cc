#include <string>
#include "FL/Fl_Native_File_Chooser.H"
#include <FL/Fl_PNG_Image.H>
#include "logo32.png.h"
#include "slog/slog.h"
#include "RustInterface.h"

#include "Launcher.h"

#define LAUNCHER_TOPIC DEBUG_TAG_LAUNCHER

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



Launcher::Launcher(const std::string exePath, engine_options_t* engine_options) : StracciatellaLauncher() {
	this->exePath = exePath;
	this->engine_options = engine_options;
}

void Launcher::show() {
	browseJa2DirectoryButton->callback((Fl_Callback *) openDataDirectorySelector, (void *) (this));
	predefinedResolutionButton->callback( (Fl_Callback*)enablePredefinedResolutionSelection, (void*)(this) );
	customResolutionButton->callback( (Fl_Callback*)enableCustomResolutionSelection, (void*)(this) );
	playButton->callback( (Fl_Callback*)startGame, (void*)(this) );
	editorButton->callback( (Fl_Callback*)startEditor, (void*)(this) );

	populateChoices();
	initializeInputsFromDefaults();

	const Fl_PNG_Image icon("logo32.png", logo32_png, 1374);
	stracciatellaLauncher->icon(&icon);
	stracciatellaLauncher->show();
}

void Launcher::initializeInputsFromDefaults() {
	char* rustResRootPath = get_vanilla_data_dir(this->engine_options);
	dataDirectoryInput->value(rustResRootPath);
	free_rust_string(rustResRootPath);

	char* rustResVersion = get_resource_version_string(get_resource_version(this->engine_options));
	gameVersionInput->value(rustResVersion);
	free_rust_string(rustResVersion);

	char resolutionString[255];
	sprintf(resolutionString, "%dx%d", get_resolution_x(this->engine_options), get_resolution_y(this->engine_options));

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

	fullscreenCheckbox->value(should_start_in_fullscreen(this->engine_options) ? 1 : 0);
	playSoundsCheckbox->value(should_start_without_sound(this->engine_options) ? 0 : 1);
}

int Launcher::writeJsonFile() {
	set_start_in_fullscreen(this->engine_options, fullscreenCheckbox->value());
	set_start_without_sound(this->engine_options, !playSoundsCheckbox->value());

	set_vanilla_data_dir(this->engine_options, dataDirectoryInput->value());

	if (customResolutionButton->value()) {
		set_resolution(this->engine_options,
						(int)customResolutionXInput->value(),
						(int)customResolutionYInput->value());
	} else {
		std::string res = predefinedResolutionInput->value();
		int split_index = res.find(RESOLUTION_SEPARATOR);
		int x = atoi(res.substr(0, split_index).c_str());
		int y = atoi(res.substr(split_index+1, res.length()).c_str());
		set_resolution(this->engine_options, x, y);
	}

	set_resource_version(this->engine_options, gameVersionInput->value());

	bool success = write_engine_options(this->engine_options);

	if (success) {
		SLOGD(LAUNCHER_TOPIC, "Succeeded writing config file");
		return 0;
	}
	SLOGD(LAUNCHER_TOPIC, "Failed writing config file");
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
		case -1:
			break; // ERROR
		case  1:
			break; // CANCEL
		default:
			window->dataDirectoryInput->value(fnfc.filename());
			break; // FILE CHOSEN
	}
}

void Launcher::enablePredefinedResolutionSelection(Fl_Widget* btn, void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );
	window->enablePredefinedResolutions();
}

void Launcher::enableCustomResolutionSelection(Fl_Widget* btn, void* userdata) {
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
