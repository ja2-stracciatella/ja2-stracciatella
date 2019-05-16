#include <string>
#include "FL/Fl_Native_File_Chooser.H"
#include <FL/Fl_PNG_Image.H>
#include <FL/fl_ask.H>
#include "logo32.png.h"
#include "slog/slog.h"
#include "RustInterface.h"

#include "Launcher.h"

#define LAUNCHER_TOPIC DEBUG_TAG_LAUNCHER

#define RESOLUTION_SEPARATOR "x"


const char* defaultResolution = "640x480";

const std::vector<GameVersion> predefinedVersions = {
	GameVersion::DUTCH,
	GameVersion::ENGLISH,
	GameVersion::FRENCH,
	GameVersion::GERMAN,
	GameVersion::ITALIAN,
	GameVersion::POLISH,
	GameVersion::RUSSIAN,
	GameVersion::RUSSIAN_GOLD
};
const std::vector< std::pair<int, int> > predefinedResolutions = {
	std::make_pair(640,  480),
	std::make_pair(800,  600),
	std::make_pair(1024, 768),
	std::make_pair(1280, 720),
	std::make_pair(1600, 900),
	std::make_pair(1920, 1080)
};
const std::vector<VideoScaleQuality> scalingModes = {
	VideoScaleQuality::LINEAR,
	VideoScaleQuality::NEAR_PERFECT,
	VideoScaleQuality::PERFECT,
};

Launcher::Launcher(const std::string exePath, EngineOptions* engine_options) : StracciatellaLauncher() {
	this->exePath = exePath;
	this->engine_options = engine_options;
}

void Launcher::show() {
	browseJa2DirectoryButton->callback((Fl_Callback *) openDataDirectorySelector, (void *) (this));
	predefinedResolutionButton->callback( (Fl_Callback*)enablePredefinedResolutionSelection, (void*)(this) );
	customResolutionButton->callback( (Fl_Callback*)enableCustomResolutionSelection, (void*)(this) );
	playButton->callback( (Fl_Callback*)startGame, (void*)(this) );
	editorButton->callback( (Fl_Callback*)startEditor, (void*)(this) );
	guessVersionButton->callback( (Fl_Callback*)guessVersion, (void*)(this) );

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

	auto rustResVersion = get_resource_version(this->engine_options);
	auto resourceVersionIndex = 0;
	for (auto version : predefinedVersions) {
		if (version == rustResVersion) {
			break;
		}
		resourceVersionIndex += 1;
	}
	gameVersionInput->value(resourceVersionIndex);

	int x = get_resolution_x(this->engine_options);
	int y = get_resolution_y(this->engine_options);
	std::pair<int, int> currentResolution = std::make_pair(x, y);

	char resolutionString[255];
	sprintf(resolutionString, "%dx%d", x, y);

	std::pair<int, int>* predefinedResolution = NULL;
	for (auto res : predefinedResolutions) {
		if (res == currentResolution) {
			predefinedResolution = &res;
		}
	}

	customResolutionXInput->value(x);
	customResolutionYInput->value(y);
	if (predefinedResolution != NULL) {
		predefinedResolutionInput->value(resolutionString);
		enablePredefinedResolutions();
	} else {
		predefinedResolutionInput->value(defaultResolution);
		enableCustomResolutions();
	}

	VideoScaleQuality quality = get_scaling_quality(this->engine_options);
	auto scalingModeIndex = 0;
	for (auto scalingMode : scalingModes) {
		if (scalingMode == quality) {
			break;
		}
		scalingModeIndex += 1;
	}
	this->scalingModeChoice->value(scalingModeIndex);

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

	auto currentResourceVersionIndex = gameVersionInput->value();
	auto currentResourceVersion = predefinedVersions.at(currentResourceVersionIndex);
	set_resource_version(this->engine_options, currentResourceVersion);

	auto currentScalingMode = scalingModes[this->scalingModeChoice->value()];
	set_scaling_quality(this->engine_options, currentScalingMode);

	bool success = write_engine_options(this->engine_options);

	if (success) {
		SLOGD(LAUNCHER_TOPIC, "Succeeded writing config file");
		return 0;
	}
	SLOGD(LAUNCHER_TOPIC, "Failed writing config file");
	return 1;
}

void Launcher::populateChoices() {
	for(GameVersion version : predefinedVersions) {
		auto resourceVersionString = get_resource_version_string(version);
		gameVersionInput->add(resourceVersionString);
		free_rust_string(resourceVersionString);
    }
	for (auto res : predefinedResolutions) {
		char resolutionString[255];
		sprintf(resolutionString, "%dx%d", res.first, res.second);
		predefinedResolutionInput->add(resolutionString);
	}

	for (auto scalingMode : scalingModes) {
		auto scalingModeString = get_scaling_quality_string(scalingMode);
		this->scalingModeChoice->add(scalingModeString);
		free_rust_string(scalingModeString);
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
	// check minimal resolution:
	if (customResolutionButton->value() &&
		(customResolutionXInput->value() < 640 ||
		customResolutionYInput->value() < 480)) {
		fl_alert("Invalid custom resolution %dx%d.\nJA2 Stracciatella needs a resolution of at least 640x480.",
			(int) customResolutionXInput->value(),
			(int) customResolutionYInput->value());
		return;
	}

	std::string cmd("\"" + this->exePath + "\"");

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

void Launcher::guessVersion(Fl_Widget* btn, void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );
	fl_message_title(window->guessVersionButton->label());
	auto choice = fl_choice("Comparing resources packs can take a long time.\nAre you sure you want to continue?", "Stop", "Continue", 0);
	if (choice != 1) {
		return;
	}

	char* log = NULL;
	auto gamedir = window->dataDirectoryInput->value();
	auto guessedVersion = guess_resource_version(gamedir, &log);
	printf("%s", log);
	if (guessedVersion != -1) {
		auto resourceVersionIndex = 0;
		for (auto version : predefinedVersions) {
			if (version == (VanillaVersion) guessedVersion) {
				break;
			}
			resourceVersionIndex += 1;
		}
		window->gameVersionInput->value(resourceVersionIndex);
		fl_message_title(window->guessVersionButton->label());
		fl_message("Success!");
	} else {
		fl_message_title(window->guessVersionButton->label());
		fl_alert("Failure!");
	}
	free_rust_string(log);
}

