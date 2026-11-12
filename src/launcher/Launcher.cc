#include "logo32.png.h"
#include "Logger.h"
#include "RustInterface.h"
#include "FileMan.h"
#include "Types.h"
#include "GameRes.h"
#include "Video.h"

#include "Launcher.h"

#include "FL/Fl_Native_File_Chooser.H"
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/fl_ask.H>
#include <string_theory/string>

#include <algorithm>
#include <vector>
#include <limits>

#define RESOLUTION_SEPARATOR "x"

const double checkGameRunningIntervalSeconds = 1.0;

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

void showError(const ST::string& error) {
	fl_message_title("Error");
	fl_alert("%s", error.c_str());
}

void showRustError() {
	RustPointer<char> err(getRustError());
	if (err) {
		SLOGE("{}", err.get());
		showError(err.get());
	} else {
		showError("showRustError called but no rust error is present");
	}
}

// FLTK draws & as an underline for the next character and @ as the start of a symbol
ST::string escapeLabel(const char* label) {
	return ST::string(label).replace("&", "&&").replace("@", "@@");
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
			SLOGE("Failed to find home directory: {}", rustError);
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
	editorButton->callback(startEditor, this);
	playButton->callback(startGame, this);
	gameDirectoryInput->callback(widgetChanged, this);
	saveGameDirectoryInput->callback(widgetChanged, this);
	browseJa2DirectoryButton->callback(openGameDirectorySelector, this);
	browseSaveGameDirectoryButton->callback(openSaveGameDirectorySelector, this);
	gameVersionInput->callback(selectGameVersion, this);
	guessVersionButton->callback(guessVersion, this);
	scalingModeChoice->callback(widgetChanged, this);
	resolutionXInput->callback(widgetChanged, this);
	resolutionYInput->callback(widgetChanged, this);
	fullscreenCheckbox->callback(widgetChanged, this);
	playSoundsCheckbox->callback(widgetChanged, this);
	gameSettingsResetAllButton->callback(resetAllGameSettings, this);
	RustPointer<char> ja2_json_path(findPathFromStracciatellaHome(this->engineOptions.get(), "ja2.json", false, true));
	if (ja2_json_path) {
		ja2JsonPathOutput->value(ja2_json_path.get());
	} else {
		ja2JsonPathOutput->value("failed to find path to ja2.json");
	}
	ja2JsonReloadBtn->callback(reloadJa2Json, this);
	ja2JsonSaveBtn->callback(saveJa2Json, this);

	auto nmods = ModManager_getAvailableModsLength(this->modManager.get());
	for (size_t i = 0; i < nmods; ++i) {
		RustPointer<Mod> mod(ModManager_getAvailableModByIndex(this->modManager.get(), i));
		RustPointer<char> modId(Mod_getId(mod.get()));
	}
	availableModsBrowser->callback(selectAvailableMods, this);
	enabledModsBrowser->callback(selectEnabledMods, this);
	enableModsButton->callback(enableMods, this);
	disableModsButton->callback(disableMods, this);
	moveDownModsButton->callback(moveDownMods, this);
	moveUpModsButton->callback(moveUpMods, this);

	populateChoices();
	initializeInputsFromDefaults();

	playButton->take_focus();

	const Fl_PNG_Image icon("logo32.png", logo32_png, 1374);
	stracciatellaLauncher->icon(&icon);
	stracciatellaLauncher->show();

	logsDisplay->buffer(logsBuffer);
	updateLogs();
}

void Launcher::initializeInputsFromDefaults() {
	RustPointer<char> rustResRootPath(EngineOptions_getVanillaGameDir(this->engineOptions.get()));
	gameDirectoryInput->value(rustResRootPath.get());
	RustPointer<char> rustSaveGamePath(EngineOptions_getSaveGameDir(this->engineOptions.get()));
	saveGameDirectoryInput->value(rustSaveGamePath.get());

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
	loadGameSettings();
	update(false);
}

void Launcher::syncModsToEngineOptions() {
	EngineOptions_clearMods(this->engineOptions.get());
	int nitems = enabledModsBrowser->size();
	for (int item = 1; item <= nitems; ++item) {
		char* modId = static_cast<char*>(enabledModsBrowser->data(item));
		EngineOptions_pushMod(this->engineOptions.get(), modId);
	}
}

// The defaults of the game settings depend on the enabled mods, so this is
// called whenever they change. Changed values are saved right away, so
// nothing is lost.
void Launcher::loadGameSettings() {
	syncModsToEngineOptions();
	gameSettings.reset(GameSettings_load(this->engineOptions.get(), this->modManager.get()));
	if (gameSettings) {
		gameSettingsError.clear();
		RustPointer<char> patchPath(GameSettings_getPatchPath(gameSettings.get()));
		gameSettingsPatchOutput->value(patchPath.get());
		gameSettingsResetAllButton->activate();
	} else {
		RustPointer<char> err(getRustError());
		gameSettingsError = err ? err.get() : "unknown error";
		SLOGE("{}", gameSettingsError);
		gameSettingsPatchOutput->value("");
		gameSettingsResetAllButton->deactivate();
	}
	buildGameSettingRows();
}

void Launcher::buildGameSettingRows() {
	gameSettingRows.clear();
	gameSettingsScroll->scroll_to(0, 0);
	gameSettingsScroll->clear();
	gameSettingsScroll->begin();

	const int rowHeight = 25;
	const int x = gameSettingsScroll->x() + 5;
	const int w = gameSettingsScroll->w() - Fl::scrollbar_size() - 10;
	const int labelWidth = w * 3 / 5;
	int y = gameSettingsScroll->y() + 5;

	size_t nsettings = gameSettings ? GameSettings_getSettingsLength(gameSettings.get()) : 0;
	if (nsettings == 0) {
		ST::string message = gameSettings
			? ST::string("There are no game settings to change.")
			: ST::format("The game settings could not be loaded: {}", gameSettingsError);
		Fl_Box* box = new Fl_Box(x, y, w, gameSettingsScroll->h() - 10);
		box->copy_label(escapeLabel(message.c_str()).c_str());
		box->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
	}

	std::optional<size_t> lastSection;
	for (size_t i = 0; i < nsettings; i++) {
		size_t section = GameSettings_getSettingSection(gameSettings.get(), i);
		if (section != lastSection) {
			if (lastSection) {
				y += 10;
			}
			RustPointer<char> sectionTitle(GameSettings_getSectionTitle(gameSettings.get(), section));
			if (sectionTitle) {
				Fl_Box* box = new Fl_Box(x, y, w, rowHeight);
				box->copy_label(escapeLabel(sectionTitle.get()).c_str());
				box->labelfont(FL_HELVETICA_BOLD);
				box->labelsize(FL_NORMAL_SIZE + 2);
				box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
				y += rowHeight;
			}
			lastSection = section;
		}

		RustPointer<char> title(GameSettings_getSettingTitle(gameSettings.get(), i));
		RustPointer<char> description(GameSettings_getSettingDescription(gameSettings.get(), i));
		RustPointer<char> allowedValues(GameSettings_getSettingAllowedValues(gameSettings.get(), i));
		RustPointer<char> value(GameSettings_getSettingValue(gameSettings.get(), i));
		ST::string tooltip = description ? description.get() : "";
		if (allowedValues) {
			if (!tooltip.empty()) {
				tooltip += "\n\n";
			}
			tooltip += ST::format("Allowed values: {}", allowedValues.get());
		}

		auto row = std::make_unique<GameSettingRow>();
		row->launcher = this;
		row->index = i;
		GameSettingType type = GameSettings_getSettingType(gameSettings.get(), i);
		if (type == GameSettingType::Boolean) {
			Fl_Check_Button* check = new Fl_Check_Button(x, y, w, rowHeight);
			// A box that draws the background, so the label can change its font
			check->box(FL_FLAT_BOX);
			check->down_box(FL_DOWN_BOX);
			check->copy_label(escapeLabel(title.get()).c_str());
			check->value(strcmp(value.get(), "true") == 0 ? 1 : 0);
			row->label = check;
			row->input = check;
		} else {
			Fl_Box* label = new Fl_Box(x, y, labelWidth, rowHeight);
			label->box(FL_FLAT_BOX);
			label->copy_label(escapeLabel(title.get()).c_str());
			label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
			int inputX = x + labelWidth + 5;
			int inputW = w - labelWidth - 5;
			Fl_Input* input;
			if (type == GameSettingType::Integer) {
				input = new Fl_Int_Input(inputX, y, inputW, rowHeight);
			} else if (type == GameSettingType::Decimal) {
				input = new Fl_Float_Input(inputX, y, inputW, rowHeight);
			} else {
				input = new Fl_Input(inputX, y, inputW, rowHeight);
			}
			input->value(value.get());
			input->when(FL_WHEN_CHANGED);
			row->label = label;
			row->input = input;
		}
		if (!tooltip.empty()) {
			row->label->copy_tooltip(tooltip.c_str());
			row->input->copy_tooltip(tooltip.c_str());
		}
		row->input->callback(gameSettingChanged, row.get());
		updateGameSettingRow(*row, true);
		gameSettingRows.push_back(std::move(row));
		y += rowHeight + 2;
	}

	gameSettingsScroll->end();
	gameSettingsScroll->redraw();
}

void Launcher::updateGameSettingRow(const GameSettingRow& row, bool valid) {
	bool modified = gameSettings && GameSettings_isSettingModified(gameSettings.get(), row.index);
	row.label->labelfont(modified ? FL_HELVETICA_BOLD : FL_HELVETICA);
	row.label->redraw();
	if (row.input != row.label) {
		row.input->color(valid ? FL_BACKGROUND2_COLOR : fl_rgb_color(255, 200, 200));
		row.input->redraw();
	}
}

void Launcher::gameSettingChanged(Fl_Widget* widget, void* userdata) {
	GameSettingRow* row = static_cast<GameSettingRow*>(userdata);
	Launcher* window = row->launcher;
	if (!window->gameSettings) {
		return;
	}
	const char* value;
	if (row->input == row->label) {
		value = static_cast<Fl_Check_Button*>(widget)->value() ? "true" : "false";
	} else {
		value = static_cast<Fl_Input*>(widget)->value();
	}
	// Invalid values are not saved, the input is highlighted until it is fixed
	bool valid = GameSettings_setSettingValue(window->gameSettings.get(), row->index, value);
	if (valid && !GameSettings_save(window->gameSettings.get())) {
		showRustError();
	}
	window->updateGameSettingRow(*row, valid);
}

void Launcher::resetAllGameSettings(Fl_Widget* btn, void* userdata) {
	Launcher* window = static_cast<Launcher*>(userdata);
	if (!window->gameSettings) {
		return;
	}
	fl_message_title("Reset All");
	if (fl_choice("Reset all game settings to their defaults?", "Cancel", "Reset", 0) != 1) {
		return;
	}
	GameSettings_resetAll(window->gameSettings.get());
	if (!GameSettings_save(window->gameSettings.get())) {
		showRustError();
	}
	window->buildGameSettingRows();
}

int Launcher::writeJsonFile() {
	EngineOptions_setStartInFullscreen(this->engineOptions.get(), fullscreenCheckbox->value());
	EngineOptions_setStartWithoutSound(this->engineOptions.get(), !playSoundsCheckbox->value());

	EngineOptions_setVanillaGameDir(this->engineOptions.get(), gameDirectoryInput->value());
	EngineOptions_setSaveGameDir(this->engineOptions.get(), saveGameDirectoryInput->value());

	syncModsToEngineOptions();

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
		update(false);
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
		ST::string resolutionString = ST::format("{d}x{d}", res.first, res.second);
		predefinedResolutionMenuButton->insert(-1, resolutionString.c_str(), 0, setPredefinedResolution, this, 0);
	}

	for (VideoScaleQuality scalingMode : scalingModes) {
		RustPointer<char> scalingModeString(ScalingQuality_toString(scalingMode));
		this->scalingModeChoice->add(scalingModeString.get());
	}
}

void Launcher::openGameDirectorySelector(Fl_Widget *btn, void *userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );
	Fl_Native_File_Chooser fnfc;
	fnfc.title("Select the original Jagged Alliance 2 installation directory");
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
			const auto dir = encodePath(fnfc.filename());
			if (!checkGameDirectoryForCommonMistakes(dir)) return;
			window->gameDirectoryInput->value(dir.c_str());
			window->update(true);
			break; // FILE CHOSEN
		}
	}
}

bool Launcher::checkGameDirectoryForCommonMistakes(const ST::string& dir) {
	auto fileToCheck = FileMan::resolveExistingComponents(FileMan::joinPaths(dir, "data/Ja2Set.dat.xml"));
	try {
		if (!checkIfRelativePathExists(dir.c_str(), "Data", true)) {
			fl_message_title("Incorrect game directory detected");
			int choice = fl_choice(
				"The Data directory was not found within game directory.\nThis means the chosen directory does not contain a Jagged Alliance 2 installation.",
				"Continue",
				"Cancel",
				0
			);
			if (choice == 1) {
				return false;
			}
		}
		if (checkIfRelativePathExists(dir.c_str(), "Data/Ja2Set.dat.xml", true)) {
			fl_message_title("Modified game directory detected");
			auto choice = fl_choice(
				"We detected that the game directory contains the 1.13 patch.\nJA2 Stracciatella will not work properly with the modified files.\nPlease use a clean installation of Jagged Alliance 2.",
				"Continue",
				"Cancel",
				0
			);

			if (choice == 1) {
				return false;
			}
		}
	} catch (const std::runtime_error &ex) {
		SLOGE("failed to read game dir: {}", ex.what());
	}
	return true;
}

void Launcher::openSaveGameDirectorySelector(Fl_Widget *btn, void *userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );
	Fl_Native_File_Chooser fnfc;
	fnfc.title("Select your save game directory");
	fnfc.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
	ST::char_buffer decoded = decodePath(window->saveGameDirectoryInput->value());
	fnfc.directory(decoded.empty() ? nullptr : decoded.c_str());

	switch ( fnfc.show() ) {
		case -1:
			break; // ERROR
		case  1:
			break; // CANCEL
		default:
		{
			ST::string encoded = encodePath(fnfc.filename());
			window->saveGameDirectoryInput->value(encoded.c_str());
			window->update(true);
			break; // FILE CHOSEN
		}
	}
}

void Launcher::startExecutable(bool asEditor) {
	if (gameIsRunning()) {
		return;
	}
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
		fl_alert("%s", message.c_str());
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
	subProcess = std::make_optional(RustPointer<SubProcess>(Subprocess_new(exePath.get(), args.get())));
	update(false);
	Launcher::maintainSubProcessState(this);
}

// Writes logs into log tab
void Launcher::updateLogs() {
	RustPointer<char> logPath(Logger_getFilePath("ja2.log"));
	try {
		AutoSGPFile logsFd (FileMan::openForReading(logPath.get()));
		auto logs = logsFd->readStringToEnd();

		logsDisplay->buffer()->text(logs.c_str());
		logsDisplay->scroll(logsBuffer.count_lines(0, logsBuffer.length()) - 1, 0);
	} catch (const std::runtime_error &ex) {
		SLOGW("Error reading logs: {}", ex.what());
	}
}

void Launcher::maintainSubProcessState(void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );
	if (window->subProcess) {
		Subprocess_process(window->subProcess.value().get());
		if (!Subprocess_isDone(window->subProcess.value().get())) {
			Fl::add_timeout(checkGameRunningIntervalSeconds, Launcher::maintainSubProcessState, window);
		} else {
			auto exitCode = Subprocess_getExitCode(window->subProcess.value().get());
			if (exitCode != 0) {
				ST::string error = "JA2 Stracciatella crashed with an unknown error.";
				if (exitCode == std::numeric_limits<std::int32_t>::min()) {
					RustPointer<char> err(getRustError());
					if (err.get() != NULL) {
						error = ST::format("JA2 Stracciatella crashed with error: {}", err.get());
					}
				} else {
					error = ST::format("JA2 Stracciatella crashed with exit code: {}", exitCode);
				}

				SLOGE("{}", error);
				error = ST::format("{}\n\nYou will be taken to the logs tab, where you can investigate the error.", error);

				showError(error);

				window->tabs->value(window->logsTab);
			}

			window->subProcess = std::nullopt;
			window->updateLogs();
			window->update(false);
		}
	}
}

bool Launcher::resolutionIsInvalid() {
	return resolutionXInput->value() < 640 || resolutionYInput->value() < 480;
}

bool Launcher::gameIsRunning() {
	return subProcess ? !Subprocess_isDone(subProcess.value().get()) : false;
}

void Launcher::update(bool changed) {
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

	if (gameIsRunning()) {
		tabs->deactivate();
		playButton->deactivate();
		editorButton->deactivate();
		ja2JsonReloadBtn->deactivate();
		ja2JsonSaveBtn->deactivate();
	} else {
		tabs->activate();
		playButton->activate();
		editorButton->activate();
		ja2JsonReloadBtn->activate();
		ja2JsonSaveBtn->activate();
	}
}

void Launcher::startGame(Fl_Widget* btn, void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );

	window->writeJsonFile();

	if (!checkGameDirectoryForCommonMistakes(window->gameDirectoryInput->value())) return;

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
	ST::string gamedir = window->gameDirectoryInput->value();

	if (!checkGameDirectoryForCommonMistakes(gamedir)) return;

	fl_message_title("Guess Game Version");
	int choice = fl_choice("Comparing resources packs can take a long time.\nAre you sure you want to continue?", "Stop", "Continue", 0);
	if (choice != 1) {
		return;
	}

	int guessedVersion = guessResourceVersion(gamedir.c_str());
	if (guessedVersion != -1) {
		int resourceVersionIndex = 0;
		for (GameVersion version : predefinedVersions) {
			if (static_cast<int>(version) == guessedVersion) {
				break;
			}
			resourceVersionIndex += 1;
		}
		window->gameVersionInput->value(resourceVersionIndex);
		window->update(true);
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
	window->update(true);
}

void Launcher::widgetChanged(Fl_Widget* widget, void* userdata) {
	Launcher* window = static_cast< Launcher* >( userdata );
	window->update(true);
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
			window->enabledModsBrowser->add(window->availableModsBrowser->text(i), window->availableModsBrowser->data(i));
			window->enabledModsBrowser->select(window->enabledModsBrowser->size());
			window->enabledModsBrowser->bottomline(window->enabledModsBrowser->size());
			window->availableModsBrowser->hide(i);
		}
	}

	if (updated) {
		window->selectEnabledMods(widget, userdata);
		window->enabledModsBrowser->redraw();
		window->availableModsBrowser->redraw();
		window->loadGameSettings();
		window->update(true);
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
		window->loadGameSettings();
		window->update(true);
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
	window->loadGameSettings();
	window->update(true);
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
	window->loadGameSettings();
	window->update(true);
}

void Launcher::selectGameVersion(Fl_Widget* widget, void* userdata)
{
	Launcher* window = static_cast<Launcher*>(userdata);
	int currentResourceVersionIndex = window->gameVersionInput->value();
	GameVersion currentResourceVersion = predefinedVersions.at(currentResourceVersionIndex);
	if (currentResourceVersion == GameVersion::SIMPLIFIED_CHINESE)
	{
		//force enable Simplified Chinese Mod
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

	window->update(true);
}
