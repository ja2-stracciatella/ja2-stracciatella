#include "logo32.png.h"
#include "Logger.h"
#include "RustInterface.h"
#include "FileMan.h"
#include "Types.h"
#include "GameRes.h"
#include "Video.h"

#include "Launcher.h"

#include "FL/Fl_Native_File_Chooser.H"
#include <FL/Fl_PNG_Image.H>
#include <FL/fl_ask.H>
#include <string_theory/string>

#include <algorithm>
#include <vector>

#define RESOLUTION_SEPARATOR "x"

const Fl_Text_Display::Style_Table_Entry styleTable[] = {
	{  FL_BLACK,		FL_COURIER_BOLD,	14 }, // A - Header
	{  FL_BLACK,		FL_COURIER,			14 }, // B - Text
	{  FL_DARK_RED,		FL_COURIER,			14 }, // B - Error Text
};

const char* defaultResolution = "640x480";

const std::vector<GameVersion> predefinedVersions = {
	GameVersion::DUTCH,
	GameVersion::ENGLISH,
	GameVersion::FRENCH,
	GameVersion::GERMAN,
	GameVersion::ITALIAN,
	GameVersion::POLISH,
	GameVersion::RUSSIAN,
	GameVersion::RUSSIAN_GOLD,
	GameVersion::SIMPLIFIED_CHINESE
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

void showRustError() {
	RustPointer<char> err(getRustError());
	if (err) {
		SLOGE("%s", err.get());
		fl_message_title("Rust error");
		fl_alert("%s", err.get());
	} else {
		RustPointer<char> err(getRustError());
		SLOGE("showRustError: no rust error");
		fl_message_title("showRustError");
		fl_alert("no rust error");
	}
}

ST::string encodePath(const char* path) {
	if (path == nullptr) {
		return ST::string();
	}
	RustPointer<char> encodedPath(Path_encodeU8(reinterpret_cast<const uint8_t*>(path), strlen(path)));
	return ST::string(encodedPath.get());
}

ST::char_buffer decodePath(const char* path) {
	if (path == nullptr) {
		return ST::char_buffer{};
	}
	ST::char_buffer buf{ST::char_buffer::strlen(path), '\0'}; // the decoded size always fits in the original size
	size_t len = Path_decodeU8(path, reinterpret_cast<uint8_t*>(buf.data()), buf.size());
	if (len > buf.size()) {
		showRustError();
		return ST::char_buffer{};
	}
	return ST::char_buffer{buf.c_str(), len};
}

Launcher::Launcher(int argc, char* argv[]) : StracciatellaLauncher() {
	this->argc = argc;
	this->argv = argv;
}

Launcher::~Launcher() {
}

void Launcher::loadJa2Json() {
	RustPointer<char> configFolderPath(EngineOptions_getStracciatellaHome());
	if (configFolderPath.get() == NULL) {
		auto rustError = getRustError();
		if (rustError != NULL) {
			SLOGE("Failed to find home directory: %s", rustError);
		}
	}

	this->engineOptions.reset(EngineOptions_create(configFolderPath.get(), argv, argc));
	this->modManager.reset(ModManager_createUnchecked(this->engineOptions.get()));

	if (this->engineOptions == NULL) {
		exit(EXIT_FAILURE);
	}
	if (EngineOptions_shouldShowHelp(this->engineOptions.get())) {
		exit(EXIT_SUCCESS);
	}
}

void Launcher::show() {
	editorButton->callback( (Fl_Callback*)startEditor, (void*)(this) );
	playButton->callback( (Fl_Callback*)startGame, (void*)(this) );
	gameDirectoryInput->callback( (Fl_Callback*)widgetChanged, (void*)(this) );
	browseJa2DirectoryButton->callback((Fl_Callback *) openGameDirectorySelector, (void *) (this));
	gameVersionInput->callback( (Fl_Callback*)selectGameVersion, (void*)(this) );
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
	RustPointer<char> ja2_json_path(findPathFromStracciatellaHome(this->engineOptions.get(), "ja2.json", false, true));
	if (ja2_json_path) {
		ja2JsonPathOutput->value(ja2_json_path.get());
	} else {
		ja2JsonPathOutput->value("failed to find path to ja2.json");
	}
	ja2JsonReloadBtn->callback( (Fl_Callback*)reloadJa2Json, (void*)(this) );
	ja2JsonSaveBtn->callback( (Fl_Callback*)saveJa2Json, (void*)(this) );

	auto nmods = ModManager_getAvailableModsLength(this->modManager.get());
	for (size_t i = 0; i < nmods; ++i) {
		RustPointer<Mod> mod(ModManager_getAvailableModByIndex(this->modManager.get(), i));
		RustPointer<char> modId(Mod_getId(mod.get()));
	}
	availableModsBrowser->callback( (Fl_Callback*)selectAvailableMods, (void*)(this) );
	enabledModsBrowser->callback( (Fl_Callback*)selectEnabledMods, (void*)(this) );
	enableModsButton->callback( (Fl_Callback*)enableMods, (void*)(this) );
	disableModsButton->callback( (Fl_Callback*)disableMods, (void*)(this) );
	moveDownModsButton->callback( (Fl_Callback*)moveDownMods, (void*)(this) );
	moveUpModsButton->callback( (Fl_Callback*)moveUpMods, (void*)(this) );

	populateChoices();
	initializeInputsFromDefaults();

	const Fl_PNG_Image icon("logo32.png", logo32_png, 1374);
	stracciatellaLauncher->icon(&icon);
	stracciatellaLauncher->show();
}

void Launcher::initializeInputsFromDefaults() {
	RustPointer<char> rustResRootPath(EngineOptions_getVanillaGameDir(this->engineOptions.get()));
	gameDirectoryInput->value(rustResRootPath.get());

	uint32_t n = EngineOptions_getModsLength(this->engineOptions.get());
	enabledModsBrowser->clear();
	for (uint32_t i = 0; i < n; ++i) {
		RustPointer<char> modId(EngineOptions_getMod(this->engineOptions.get(), i));
		RustPointer<Mod> mod(ModManager_getAvailableModById(this->modManager.get(), modId.get()));
		if (mod.get() != NULL) {
			RustPointer<char> modName(Mod_getName(mod.get()));

			enabledModsBrowser->add(modName.get());
			enabledModsBrowser->data(enabledModsBrowser->size(), modId.release());
		} else {
			// @C72 is dark red, should be highlighted because it is not available
			enabledModsBrowser->add(ST::format("@C72{}", modId.get()).c_str());
			enabledModsBrowser->data(enabledModsBrowser->size(), modId.release());
		}
	}

	availableModsBrowser->clear();
	auto nmods = ModManager_getAvailableModsLength(this->modManager.get());
	for (uintptr_t i = 0; i < nmods; i++) {
		RustPointer<Mod> mod(ModManager_getAvailableModByIndex(this->modManager.get(), i));
		RustPointer<char> modId(Mod_getId(mod.get()));
		RustPointer<char> modName(Mod_getName(mod.get()));
		availableModsBrowser->add(modName.get());
		if (EngineOptions_isModEnabled(this->engineOptions.get(), modId.get())) {
			availableModsBrowser->hide(availableModsBrowser->size());
		}
		availableModsBrowser->data(availableModsBrowser->size(), modId.release());
	}

	GameVersion rustResVersion = EngineOptions_getResourceVersion(this->engineOptions.get());
	int resourceVersionIndex = 0;
	for (GameVersion version : predefinedVersions) {
		if (version == rustResVersion) {
			break;
		}
		resourceVersionIndex += 1;
	}
	gameVersionInput->value(resourceVersionIndex);

	int x = EngineOptions_getResolutionX(this->engineOptions.get());
	int y = EngineOptions_getResolutionY(this->engineOptions.get());

	resolutionXInput->value(x);
	resolutionYInput->value(y);

	VideoScaleQuality quality = EngineOptions_getScalingQuality(this->engineOptions.get());
	int scalingModeIndex = 0;
	for (VideoScaleQuality scalingMode : scalingModes) {
		if (scalingMode == quality) {
			break;
		}
		scalingModeIndex += 1;
	}
	this->scalingModeChoice->value(scalingModeIndex);

	fullscreenCheckbox->value(EngineOptions_shouldStartInFullscreen(this->engineOptions.get()) ? 1 : 0);
	playSoundsCheckbox->value(EngineOptions_shouldStartWithoutSound(this->engineOptions.get()) ? 0 : 1);
	update(false, nullptr);
}

int Launcher::writeJsonFile() {
	EngineOptions_setStartInFullscreen(this->engineOptions.get(), fullscreenCheckbox->value());
	EngineOptions_setStartWithoutSound(this->engineOptions.get(), !playSoundsCheckbox->value());

	EngineOptions_setVanillaGameDir(this->engineOptions.get(), gameDirectoryInput->value());

	EngineOptions_clearMods(this->engineOptions.get());
	int nitems = enabledModsBrowser->size();
	for (int item = 1; item <= nitems; ++item) {
		char* modId = static_cast<char*>(enabledModsBrowser->data(item));
		EngineOptions_pushMod(this->engineOptions.get(), modId);
	}

	int x = (int)resolutionXInput->value();
	int y = (int)resolutionYInput->value();
	EngineOptions_setResolution(this->engineOptions.get(), x, y);
	EngineOptions_setBrightness(this->engineOptions.get(), -1.0f);

	int currentResourceVersionIndex = gameVersionInput->value();
	GameVersion currentResourceVersion = predefinedVersions.at(currentResourceVersionIndex);
	EngineOptions_setResourceVersion(this->engineOptions.get(), currentResourceVersion);

	VideoScaleQuality currentScalingMode = scalingModes[this->scalingModeChoice->value()];
	EngineOptions_setScalingQuality(this->engineOptions.get(), currentScalingMode);

	bool success = EngineOptions_write(this->engineOptions.get());

	if (success) {
		update(false, nullptr);
		SLOGD("Succeeded writing config file");
		return 0;
	}
	SLOGD("Failed writing config file");
	return 1;
}

void Launcher::populateChoices() {
	for(GameVersion version : predefinedVersions) {
		RustPointer<char> resourceVersionString(VanillaVersion_toString(version));
		gameVersionInput->add(resourceVersionString.get());
	}
	for (std::pair<int,int> res : predefinedResolutions) {
		char resolutionString[255];
		sprintf(resolutionString, "%dx%d", res.first, res.second);
		predefinedResolutionMenuButton->insert(-1, resolutionString, 0, setPredefinedResolution, this, 0);
	}

	for (VideoScaleQuality scalingMode : scalingModes) {
		RustPointer<char> scalingModeString(ScalingQuality_toString(scalingMode));
		this->scalingModeChoice->add(scalingModeString.get());
	}
}

void Launcher::openGameDirectorySelector(Fl_Widget *btn, void *userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );
	Fl_Native_File_Chooser fnfc;
	fnfc.title("Select the original Jagged Alliance 2 install directory");
	fnfc.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
	ST::char_buffer decoded = decodePath(window->gameDirectoryInput->value());
	fnfc.directory(decoded.empty() ? nullptr : decoded.c_str());

	switch ( fnfc.show() ) {
		case -1:
			break; // ERROR
		case  1:
			break; // CANCEL
		default:
		{
			ST::string encoded = encodePath(fnfc.filename());
			window->gameDirectoryInput->value(encoded.c_str());
			window->update(true, window->gameDirectoryInput);
			break; // FILE CHOSEN
		}
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

	auto nenabled = this->enabledModsBrowser->size();
	std::vector<ST::string> invalidMods;
	for (auto i = 1; i <= nenabled; i++) {
		ST::string modId = static_cast<char*>(this->enabledModsBrowser->data(i));
		if (ModManager_getAvailableModById(this->modManager.get(), modId.c_str()) == NULL) {
			invalidMods.push_back(modId);
		}
	}
	if (invalidMods.size() > 0) {
		ST::string message = "The following mods are enabled, but dont exist on the filesystem: ";
		for (auto i = invalidMods.begin(); i < invalidMods.end(); i++) {
			if (i != invalidMods.begin()) {
				message += ", ";
			}
			message += *i;
		}

		fl_message_title("Invalid mods");
		fl_alert(message.c_str());
		return;
	}

	RustPointer<char> exePath(Env_currentExe());
	if (!exePath) {
		showRustError();
		return;
	}
	ST::string filename = FileMan::getFileName(exePath.get());
	if (filename.size() == 0) {
		fl_message_title("No filename");
		fl_alert("%s", exePath.get());
		return;
	}
	ST::string target("-launcher");
	ST::string newFilename(filename);
	auto pos = newFilename.find_last(target);
	if (pos == -1) {
		fl_message_title("Not launcher");
		fl_alert("%s", exePath.get());
		return;
	}
	newFilename = newFilename.replace(target, "");
	exePath.reset(Path_setFilename(exePath.get(), newFilename.c_str()));
	if (!FileMan::exists(exePath.get())) {
		fl_message_title("Not found");
		fl_alert("%s", exePath.get());
		return;
	}
	RustPointer<VecCString> args(VecCString_create());
	if (asEditor) {
		VecCString_push(args.get(), "-editor");
	}
	bool ok = Command_execute(exePath.get(), args.get());
	if (!ok) {
		showRustError();
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
		ST::string tmp("*"); // add '*'
		tmp += ja2JsonPathOutput->value();
		ja2JsonPathOutput->value(tmp.c_str());
	} else if (!changed && ja2JsonPathOutput->value()[0] == '*') {
		ST::string tmp(ja2JsonPathOutput->value() + 1); // remove '*'
		ja2JsonPathOutput->value(tmp.c_str());
	}
}

void Launcher::startGame(Fl_Widget* btn, void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );

	window->writeJsonFile();
	if (!checkIfRelativePathExists(window->gameDirectoryInput->value(), "Data", true)) {
		fl_message_title(window->playButton->label());
		int choice = fl_choice("Data dir not found.\nAre you sure you want to continue?", "Stop", "Continue", 0);
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
			has_editor_slf = checkIfRelativePathExists(assets_dir.get(), "externalized/editor.slf", true);
		}
	}
	if (!has_editor_slf) {
		fl_message_title(window->editorButton->label());
		int choice = fl_choice("Editor.slf not found.\nAre you sure you want to continue?", "Stop", "Continue", 0);
		if (choice != 1) {
			return;
		}
	}
	window->startExecutable(true);
}

void Launcher::guessVersion(Fl_Widget* btn, void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );
	fl_message_title("Guess Game Version");
	int choice = fl_choice("Comparing resources packs can take a long time.\nAre you sure you want to continue?", "Stop", "Continue", 0);
	if (choice != 1) {
		return;
	}

	const char* gamedir = window->gameDirectoryInput->value();
	int guessedVersion = guessResourceVersion(gamedir);
	if (guessedVersion != -1) {
		int resourceVersionIndex = 0;
		for (GameVersion version : predefinedVersions) {
			if (static_cast<int>(version) == guessedVersion) {
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
	ST::string res = menuBtn->mvalue()->label();
	int x = 0;
	int y = 0;
	(void)sscanf(res.c_str(), "%d" RESOLUTION_SEPARATOR "%d", &x, &y);
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

void Launcher::showModDetails(const ST::string& modId) {
	int styleTableSize = sizeof(styleTable)/sizeof(styleTable[0]);
	Fl_Text_Buffer *textBuffer = new Fl_Text_Buffer();
    Fl_Text_Buffer *styleBuffer = new Fl_Text_Buffer();
	RustPointer<Mod> mod(ModManager_getAvailableModById(this->modManager.get(), modId.c_str()));
	if (mod.get() != NULL) {
		ST::string modName(RustPointer<char>(Mod_getName(mod.get())).get());
		ST::string modVersion(RustPointer<char>(Mod_getVersionString(mod.get())).get());
		ST::string modDescription(RustPointer<char>(Mod_getDescription(mod.get())).get());

		auto nameLine = ST::format("{}\n", modName);
		auto versionLine = ST::format("Version: {}\n\n", modVersion);
		auto description = ST::string("");
		if (!modDescription.empty()) {
			description = ST::format("{}\n\n", modDescription);
		}
		auto idLine = ST::format("Mod Id: {}", modId);

		auto modDetails = nameLine + versionLine + description + idLine;
		ST::string modDetailsStyle;
		for (size_t i = 0; i < nameLine.size(); i++) {
			modDetailsStyle += "A";
		}
		for (size_t i = 0; i < (versionLine.size() + description.size() + idLine.size()); i++) {
			modDetailsStyle += "B";
		}

		textBuffer->text(modDetails.c_str());
		styleBuffer->text(modDetailsStyle.c_str());
	} else {
		auto error = ST::format("Error: Could not find mod '{}'", modId);

		ST::string errorStyle;
		for (size_t i = 0; i < error.size(); i++) {
			errorStyle += "C";
		}

		textBuffer->text(error.c_str());
		styleBuffer->text(errorStyle.c_str());
	}

	this->modDetails->buffer(textBuffer);
	this->modDetails->highlight_data(styleBuffer, styleTable, styleTableSize, 'A', 0, 0);
	this->modDetails->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
	this->modDetails->show();
}

void Launcher::hideModDetails() {
	this->modDetails->hide();
}

void Launcher::selectAvailableMods(Fl_Widget* widget, void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );

	window->enableModsButton->activate();
	window->disableModsButton->deactivate();
	window->moveUpModsButton->deactivate();
	window->moveDownModsButton->deactivate();
	auto nenabled = window->enabledModsBrowser->size();
	for (auto i = 1; i <= nenabled; i++) {
		window->enabledModsBrowser->select(i, 0);
	}
	std::vector<ST::string> selectedMods;
	auto navailable = window->availableModsBrowser->size();
	for (auto i = 1; i <= navailable; i++) {
		if (window->availableModsBrowser->visible(i) && window->availableModsBrowser->selected(i)) {
			selectedMods.push_back(ST::string(static_cast<char*>(window->availableModsBrowser->data(i))));
		}
	}
	if (selectedMods.size() == 1) {
		window->showModDetails(selectedMods[0]);
	} else {
		window->hideModDetails();
	}
}

void Launcher::selectEnabledMods(Fl_Widget* widget, void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );

	window->enableModsButton->deactivate();
	window->disableModsButton->activate();
	window->moveUpModsButton->activate();
	window->moveDownModsButton->activate();
	auto navailable = window->availableModsBrowser->size();
	for (auto i = 1; i <= navailable; i++) {
		window->availableModsBrowser->select(i, 0);
	}
	std::vector<ST::string> selectedMods;
	auto nenabled = window->enabledModsBrowser->size();
	for (auto i = 1; i <= nenabled; i++) {
		if (window->enabledModsBrowser->visible(i) && window->enabledModsBrowser->selected(i)) {
			selectedMods.push_back(ST::string(static_cast<char*>(window->enabledModsBrowser->data(i))));
		}
	}
	if (selectedMods.size() == 1) {
		window->showModDetails(selectedMods[0]);
	} else {
		window->hideModDetails();
	}
}

void Launcher::enableMods(Fl_Widget* widget, void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );

	bool updated = false;
	for (auto i = window->availableModsBrowser->size(); i > 0; i--) {
		if (window->availableModsBrowser->selected(i) && window->availableModsBrowser->visible(i)) {
			updated = true;
			window->enabledModsBrowser->insert(1, window->availableModsBrowser->text(i));
			window->enabledModsBrowser->data(1, window->availableModsBrowser->data(i));
			window->enabledModsBrowser->select(1, 1);
			window->availableModsBrowser->hide(i);
		}
	}

	if (updated) {
		window->selectEnabledMods(widget, userdata);
		window->enabledModsBrowser->redraw();
		window->availableModsBrowser->redraw();
		window->update(true, widget);
	}
}

void Launcher::disableMods(Fl_Widget* widget, void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );

	bool updated = false;
	for (auto i = window->enabledModsBrowser->size(); i > 0; i--) {
		if (window->enabledModsBrowser->selected(i)) {
			updated = true;

			auto id = ST::string(static_cast<char*>(window->enabledModsBrowser->data(i)));
			window->enabledModsBrowser->remove(i);
			for (auto j = window->availableModsBrowser->size(); j > 0; j--) {
				auto otherId = ST::string(static_cast<char*>(window->availableModsBrowser->data(j)));
				if (id == otherId) {
					window->availableModsBrowser->show(j);
					window->availableModsBrowser->select(j, 1);
				}
			}
		}
	}

	if (updated) {
		window->selectAvailableMods(widget, userdata);
		window->enabledModsBrowser->redraw();
		window->availableModsBrowser->redraw();
		window->update(true, widget);
	}
}

void Launcher::moveUpMods(Fl_Widget* widget, void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );
	int nitems = window->enabledModsBrowser->size();

	if (nitems <= 1) {
		return;
	}

	// Fltk line indexing is 1 based
	for (auto i = 2; i <= nitems; i++) {
		if (window->enabledModsBrowser->selected(i) && !window->enabledModsBrowser->selected(i-1)) {
			window->enabledModsBrowser->swap(i, i-1);
		}
	}

	window->enabledModsBrowser->redraw();
	window->update(true, widget);
}

void Launcher::moveDownMods(Fl_Widget* widget, void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );
	int nitems = window->enabledModsBrowser->size();

	if (nitems <= 1) {
		return;
	}

	// Fltk line indexing is 1 based
	for (auto i = nitems - 1; i > 0; i--) {
		if (window->enabledModsBrowser->selected(i) && !window->enabledModsBrowser->selected(i+1)) {
			window->enabledModsBrowser->swap(i, i+1);
		}
	}

	window->enabledModsBrowser->redraw();
	window->update(true, widget);
}

void Launcher::selectGameVersion(Fl_Widget* widget, void* userdata)
{
	Launcher* window = static_cast<Launcher*>(userdata);
	int currentResourceVersionIndex = window->gameVersionInput->value();
	GameVersion currentResourceVersion = predefinedVersions.at(currentResourceVersionIndex);
	if (currentResourceVersion == GameVersion::SIMPLIFIED_CHINESE)
	{
		//force enalbe Simplified Chinese Mod
		for (auto i = window->availableModsBrowser->size(); i > 0; i--)
		{
			char* modId = static_cast<char*>(window->availableModsBrowser->data(i));
			window->availableModsBrowser->select(i, strcmp(modId, SIMPLIFIED_CHINESE_MOD_NAME) == 0 ? 1 : 0);
		}
		enableMods(window->enableModsButton, userdata);
	}
	else
	{
		//force diable Simplified Chinese Mod
		for (auto i = window->enabledModsBrowser->size(); i > 0; i--)
		{
			char* modId = static_cast<char*>(window->enabledModsBrowser->data(i));
			window->enabledModsBrowser->select(i, strcmp(modId, SIMPLIFIED_CHINESE_MOD_NAME) == 0 ? 1 : 0);
		}
		disableMods(window->disableModsButton, userdata);
	}

	window->update(true, widget);
}
