#include <algorithm>
#include <string>
#include <vector>
#include "FL/Fl_Native_File_Chooser.H"
#include <FL/Fl_PNG_Image.H>
#include <FL/fl_ask.H>
#include "logo32.png.h"
#include "Logger.h"
#include "RustInterface.h"
#include "Types.h"
#include "GameRes.h"
#include "Video.h"

#include "Launcher.h"

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

Launcher::Launcher(int argc, char* argv[]) : StracciatellaLauncher() {
	this->argc = argc;
	this->argv = argv;
}

Launcher::~Launcher() {
}

void Launcher::loadJa2Json() {
	RustPointer<char> rustExePath(findJa2Executable(argv[0]));
	this->exePath = std::string(rustExePath.get());

	this->engine_options.reset(EngineOptions_create(argv, argc));

	if (this->engine_options == NULL) {
		exit(EXIT_FAILURE);
	}
	if (EngineOptions_shouldShowHelp(this->engine_options.get())) {
		exit(EXIT_SUCCESS);
	}
}

void Launcher::show() {
	editorButton->callback( (Fl_Callback*)startEditor, (void*)(this) );
	playButton->callback( (Fl_Callback*)startGame, (void*)(this) );
	gameDirectoryInput->callback( (Fl_Callback*)widgetChanged, (void*)(this) );
	browseJa2DirectoryButton->callback((Fl_Callback *) openGameDirectorySelector, (void *) (this));
	gameVersionInput->callback( (Fl_Callback*)widgetChanged, (void*)(this) );
	guessVersionButton->callback( (Fl_Callback*)guessVersion, (void*)(this) );
	scalingModeChoice->callback( (Fl_Callback*)widgetChanged, (void*)(this) );
	resolutionXInput->callback( (Fl_Callback*)widgetChanged, (void*)(this) );
	resolutionYInput->callback( (Fl_Callback*)widgetChanged, (void*)(this) );
	RustPointer<char> game_json_path(findPathFromAssetsDir("externalized/game.json", true, true));
	if (game_json_path) {
		gameSettingsOutput->value(game_json_path.get());
	} else {
		gameSettingsOutput->value("failed to find path to game.json");
	}
	fullscreenCheckbox->callback( (Fl_Callback*)widgetChanged, (void*)(this) );
	playSoundsCheckbox->callback( (Fl_Callback*)widgetChanged, (void*)(this) );
	RustPointer<char> ja2_json_path(findPathFromStracciatellaHome("ja2.json", false, true));
	if (ja2_json_path) {
		ja2JsonPathOutput->value(ja2_json_path.get());
	} else {
		ja2JsonPathOutput->value("failed to find path to ja2.json");
	}
	ja2JsonReloadBtn->callback( (Fl_Callback*)reloadJa2Json, (void*)(this) );
	ja2JsonSaveBtn->callback( (Fl_Callback*)saveJa2Json, (void*)(this) );
	addModMenuButton->callback( (Fl_Callback*)addMod, (void*)(this) );
	moveDownModsButton->callback( (Fl_Callback*)moveDownMods, (void*)(this) );
	moveUpModsButton->callback( (Fl_Callback*)moveUpMods, (void*)(this) );
	removeModsButton->callback( (Fl_Callback*)removeMods, (void*)(this) );

	populateChoices();
	initializeInputsFromDefaults();

	const Fl_PNG_Image icon("logo32.png", logo32_png, 1374);
	stracciatellaLauncher->icon(&icon);
	stracciatellaLauncher->show();
}

void Launcher::initializeInputsFromDefaults() {
	RustPointer<char> rustResRootPath(EngineOptions_getVanillaGameDir(this->engine_options.get()));
	gameDirectoryInput->value(rustResRootPath.get());

	auto n = EngineOptions_getModsLength(this->engine_options.get());
	modsCheckBrowser->clear();
	for (auto i = 0; i < n; ++i) {
		modsCheckBrowser->add(EngineOptions_getMod(this->engine_options.get(), i));
	}

	auto rustResVersion = EngineOptions_getResourceVersion(this->engine_options.get());
	auto resourceVersionIndex = 0;
	for (auto version : predefinedVersions) {
		if (version == rustResVersion) {
			break;
		}
		resourceVersionIndex += 1;
	}
	gameVersionInput->value(resourceVersionIndex);

	int x = EngineOptions_getResolutionX(this->engine_options.get());
	int y = EngineOptions_getResolutionY(this->engine_options.get());

	resolutionXInput->value(x);
	resolutionYInput->value(y);

	VideoScaleQuality quality = EngineOptions_getScalingQuality(this->engine_options.get());
	auto scalingModeIndex = 0;
	for (auto scalingMode : scalingModes) {
		if (scalingMode == quality) {
			break;
		}
		scalingModeIndex += 1;
	}
	this->scalingModeChoice->value(scalingModeIndex);

	fullscreenCheckbox->value(EngineOptions_shouldStartInFullscreen(this->engine_options.get()) ? 1 : 0);
	playSoundsCheckbox->value(EngineOptions_shouldStartWithoutSound(this->engine_options.get()) ? 0 : 1);
	update(false, nullptr);
}

int Launcher::writeJsonFile() {
	EngineOptions_setStartInFullscreen(this->engine_options.get(), fullscreenCheckbox->value());
	EngineOptions_setStartWithoutSound(this->engine_options.get(), !playSoundsCheckbox->value());

	EngineOptions_setVanillaGameDir(this->engine_options.get(), gameDirectoryInput->value());

	EngineOptions_clearMods(this->engine_options.get());
	auto nitems = modsCheckBrowser->nitems();
	for (auto item = 1; item <= nitems; ++item) {
		EngineOptions_pushMod(this->engine_options.get(), modsCheckBrowser->text(item));
	}

	int x = (int)resolutionXInput->value();
	int y = (int)resolutionYInput->value();
	EngineOptions_setResolution(this->engine_options.get(), x, y);

	auto currentResourceVersionIndex = gameVersionInput->value();
	auto currentResourceVersion = predefinedVersions.at(currentResourceVersionIndex);
	EngineOptions_setResourceVersion(this->engine_options.get(), currentResourceVersion);

	auto currentScalingMode = scalingModes[this->scalingModeChoice->value()];
	EngineOptions_setScalingQuality(this->engine_options.get(), currentScalingMode);

	bool success = EngineOptions_write(this->engine_options.get());

	if (success) {
		update(false, nullptr);
		SLOGD("Succeeded writing config file");
		return 0;
	}
	SLOGD("Failed writing config file");
	return 1;
}

void Launcher::populateChoices() {
	RustPointer<VecCString> mods(findAvailableMods());
	auto nmods = VecCString_length(mods.get());
	for (auto i = 0; i < nmods; ++i) {
		RustPointer<char> mod(VecCString_get(mods.get(), i));
		addModMenuButton->insert(-1, mod.get(), 0, addMod, this, 0);
	}

	for(GameVersion version : predefinedVersions) {
		RustPointer<char> resourceVersionString(VanillaVersion_toString(version));
		gameVersionInput->add(resourceVersionString.get());
    }
	for (auto res : predefinedResolutions) {
		char resolutionString[255];
		sprintf(resolutionString, "%dx%d", res.first, res.second);
		predefinedResolutionMenuButton->insert(-1, resolutionString, 0, setPredefinedResolution, this, 0);
	}

	for (auto scalingMode : scalingModes) {
		RustPointer<char> scalingModeString(ScalingQuality_toString(scalingMode));
		this->scalingModeChoice->add(scalingModeString.get());
	}
}

void Launcher::openGameDirectorySelector(Fl_Widget *btn, void *userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );
	Fl_Native_File_Chooser fnfc;
	fnfc.title("Select the original Jagged Alliance 2 install directory");
	fnfc.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
	fnfc.directory(window->gameDirectoryInput->value());

	switch ( fnfc.show() ) {
		case -1:
			break; // ERROR
		case  1:
			break; // CANCEL
		default:
			window->gameDirectoryInput->value(fnfc.filename());
			window->update(true, window->gameDirectoryInput);
			break; // FILE CHOSEN
	}
}

void Launcher::startExecutable(bool asEditor) {
	// check minimal resolution:
	if (resolutionIsInvalid()) {
		fl_message_title("Invalid resolution");
		fl_alert("Invalid custom resolution %dx%d.\nJA2 Stracciatella needs a resolution of at least 640x480.",
			(int) resolutionXInput->value(),
			(int) resolutionYInput->value());
		return;
	}

	std::string cmd("\"" + this->exePath + "\"");

	if (asEditor) {
		cmd += std::string(" -editor");
	}

	int ret = system(cmd.c_str());
	if (ret != 0)
	{
		SLOGW("There was an error while running '%s' (%d)", cmd.c_str(), ret);
	}
}

bool Launcher::resolutionIsInvalid() {
	return resolutionXInput->value() < 640 || resolutionYInput->value() < 480;
}

void Launcher::update(bool changed, Fl_Widget *widget) {
	// invalid resolution warning
	if (resolutionIsInvalid()) {
		invalidResolutionLabel->show();
	} else {
		invalidResolutionLabel->hide();
	}

	// something changed indicator
	if (changed && ja2JsonPathOutput->value()[0] != '*') {
		std::string tmp("*"); // add '*'
		tmp += ja2JsonPathOutput->value();
		ja2JsonPathOutput->value(tmp.c_str());
	} else if (!changed && ja2JsonPathOutput->value()[0] == '*') {
		std::string tmp(ja2JsonPathOutput->value() + 1); // remove '*'
		ja2JsonPathOutput->value(tmp.c_str());
	}
}

void Launcher::startGame(Fl_Widget* btn, void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );

	window->writeJsonFile();
	if (!checkIfRelativePathExists(window->gameDirectoryInput->value(), "Data", true)) {
		fl_message_title(window->playButton->label());
		auto choice = fl_choice("Data dir not found.\nAre you sure you want to continue?", "Stop", "Continue", 0);
		if (choice != 1) {
			return;
		}
	}
	window->startExecutable(false);
}

void Launcher::startEditor(Fl_Widget* btn, void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );

	window->writeJsonFile();
	bool has_editor_slf = checkIfRelativePathExists(window->gameDirectoryInput->value(), "Data/Editor.slf", true);
	if (!has_editor_slf) {
		RustPointer<char> assets_dir(findPathFromAssetsDir(nullptr, false, false));
		if (assets_dir) {
			// free editor.slf
			has_editor_slf = checkIfRelativePathExists(assets_dir.get(), "editor.slf", true);
		}
	}
	if (!has_editor_slf) {
		fl_message_title(window->editorButton->label());
		auto choice = fl_choice("Editor.slf not found.\nAre you sure you want to continue?", "Stop", "Continue", 0);
		if (choice != 1) {
			return;
		}
	}
	window->startExecutable(true);
}

void Launcher::guessVersion(Fl_Widget* btn, void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );
	fl_message_title("Guess Game Version");
	auto choice = fl_choice("Comparing resources packs can take a long time.\nAre you sure you want to continue?", "Stop", "Continue", 0);
	if (choice != 1) {
		return;
	}

	auto gamedir = window->gameDirectoryInput->value();
	auto guessedVersion = guessResourceVersion(gamedir);
	if (guessedVersion != -1) {
		auto resourceVersionIndex = 0;
		for (auto version : predefinedVersions) {
			if (version == (VanillaVersion) guessedVersion) {
				break;
			}
			resourceVersionIndex += 1;
		}
		window->gameVersionInput->value(resourceVersionIndex);
		window->update(true, window->gameVersionInput);
		fl_message_title(window->guessVersionButton->label());
		fl_message("Success!");
	} else {
		fl_message_title(window->guessVersionButton->label());
		fl_alert("Failure!");
	}
}

void Launcher::setPredefinedResolution(Fl_Widget* btn, void* userdata) {
	Fl_Menu_Button* menuBtn = static_cast< Fl_Menu_Button* >( btn );
	Launcher* window = static_cast< Launcher* >( userdata );
	std::string res = menuBtn->mvalue()->label();
	size_t split_index = res.find(RESOLUTION_SEPARATOR);
	int x = atoi(res.substr(0, split_index).c_str());
	int y = atoi(res.substr(split_index+1, res.length()).c_str());
	window->resolutionXInput->value(x);
	window->resolutionYInput->value(y);
	window->update(true, btn);
}

void Launcher::widgetChanged(Fl_Widget* widget, void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );
	window->update(true, widget);
}

void Launcher::reloadJa2Json(Fl_Widget* widget, void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );
	window->loadJa2Json();
	window->initializeInputsFromDefaults();
}

void Launcher::saveJa2Json(Fl_Widget* widget, void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );
	window->writeJsonFile();
}

void Launcher::addMod(Fl_Widget* widget, void* userdata) {
	Fl_Menu_Button* menuButton = static_cast< Fl_Menu_Button* >( widget );
	Launcher* window = static_cast< Launcher* >( userdata );

	auto mod = menuButton->mvalue()->label();
	window->modsCheckBrowser->add(mod);
	window->modsCheckBrowser->redraw();
	window->update(true, widget);
}

void Launcher::moveUpMods(Fl_Widget* widget, void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );
	auto nitems = window->modsCheckBrowser->nitems();
	auto nchecked = window->modsCheckBrowser->nchecked();
	if (nchecked == 0 || nchecked == nitems) {
		return; // nothing to do
	}

	std::vector<int> order;
	for (auto item = 1; item <= nitems; ++item) {
		if (window->modsCheckBrowser->checked(item)) {
			if (!order.empty() && !window->modsCheckBrowser->checked(order.back())) {
				order.insert(order.end() - 1, item); // move up
				continue;
			}
		}
		order.emplace_back(item);
	}

	std::vector<std::string> text;
	std::vector<int> checked;
	for (auto item : order) {
		text.emplace_back(window->modsCheckBrowser->text(item));
		checked.emplace_back(window->modsCheckBrowser->checked(item));
	}

	window->modsCheckBrowser->clear();
	for (auto i = 0; i < nitems; ++i) {
		window->modsCheckBrowser->add(text[i].c_str(), checked[i]);
	}
	window->update(true, widget);
}

void Launcher::moveDownMods(Fl_Widget* widget, void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );
	auto nitems = window->modsCheckBrowser->nitems();
	auto nchecked = window->modsCheckBrowser->nchecked();
	if (nchecked == 0 || nchecked == nitems) {
		return; // nothing to do
	}

	std::vector<int> order;
	for (auto item = nitems; item >= 1; --item) {
		if (window->modsCheckBrowser->checked(item)) {
			if (!order.empty() && !window->modsCheckBrowser->checked(order.back())) {
				order.insert(order.end() - 1, item); // move down
				continue;
			}
		}
		order.emplace_back(item);
	}
	std::reverse(order.begin(), order.end());

	std::vector<std::string> text;
	std::vector<int> checked;
	for (auto item : order) {
		text.emplace_back(window->modsCheckBrowser->text(item));
		checked.emplace_back(window->modsCheckBrowser->checked(item));
	}

	window->modsCheckBrowser->clear();
	for (auto i = 0; i < nitems; ++i) {
		window->modsCheckBrowser->add(text[i].c_str(), checked[i]);
	}
	window->update(true, widget);
}

void Launcher::removeMods(Fl_Widget* widget, void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );
	auto nchecked = window->modsCheckBrowser->nchecked();
	if (nchecked == 0) {
		return; // nothing to do
	}

	std::vector<std::string> text;
	auto nitems = window->modsCheckBrowser->nitems();
	for (auto item = 1; item <= nitems; ++item) {
		if (!window->modsCheckBrowser->checked(item)) {
			text.emplace_back(window->modsCheckBrowser->text(item));
		}
	}

	window->modsCheckBrowser->clear();
	for (auto i = 0; i < text.size(); ++i) {
		window->modsCheckBrowser->add(text[i].c_str());
	}
	window->update(true, widget);
}
