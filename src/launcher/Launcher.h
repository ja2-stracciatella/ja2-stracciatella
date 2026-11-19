#ifndef JA2_LAUNCHER_H_H
#define JA2_LAUNCHER_H_H

#include "StracciatellaLauncher.h"
#include "RustInterface.h"

#include <string_theory/string>

#include <iostream>
#include <iterator>
#include <memory>
#include <set>
#include <optional>
#include <vector>

struct sortMods {
    bool operator() (ST::string a, ST::string b) const {
        return a.compare(b) < 0;
    }
};

#define SIMPLIFIED_CHINESE_MOD_NAME "simplified-chinese-localization"

class Launcher;

// The widgets of a single setting in the game settings tab
struct GameSettingRow {
	Launcher* launcher;
	size_t index;
	// The widget that shows the title, the input itself for check buttons
	Fl_Widget* label;
	Fl_Widget* input;
};

class Launcher : public StracciatellaLauncher {
public:
	Launcher(int argc, char* argv[]);
	~Launcher();

	void loadJa2Json();
	void show();
	void initializeInputsFromDefaults();
	int writeJsonFile();
private:
	int argc;
	char** argv;
	RustPointer<EngineOptions> engineOptions;
	RustPointer<ModManager> modManager;
	std::optional<RustPointer<SubProcess>> subProcess;
	Fl_Text_Buffer logsBuffer;
	RustPointer<GameSettings> gameSettings;
	std::vector<std::unique_ptr<GameSettingRow>> gameSettingRows;
	// Why the game settings could not be loaded, if they could not
	ST::string gameSettingsError;

	void populateChoices();
	void startExecutable(bool asEditor);
	bool resolutionIsInvalid();
	bool gameIsRunning();
	void update(bool changed);
	void updateLogs();
	void showModDetails(const ST::string& modName);
	void hideModDetails();
	void syncModsToEngineOptions();
	void loadGameSettings();
	void buildGameSettingRows();
	void updateGameSettingRow(const GameSettingRow& row, bool valid);
	static bool checkGameDirectoryForCommonMistakes(const ST::string& gameDir);
	static void openGameDirectorySelector(Fl_Widget *btn, void *userdata);
	static void openSaveGameDirectorySelector(Fl_Widget *btn, void *userdata);
	static void gameSettingChanged(Fl_Widget* widget, void* userdata);
	static void resetAllGameSettings(Fl_Widget* btn, void* userdata);
	static void startGame(Fl_Widget* btn, void* userdata);
	static void startEditor(Fl_Widget* btn, void* userdata);
	static void guessVersion(Fl_Widget* btn, void* userdata);
	static void setPredefinedResolution(Fl_Widget* btn, void* userdata);
	static void widgetChanged(Fl_Widget* widget, void* userdata);
	static void reloadJa2Json(Fl_Widget* widget, void* userdata);
	static void saveJa2Json(Fl_Widget* widget, void* userdata);
	static void selectEnabledMods(Fl_Widget* widget, void* userdata);
	static void selectAvailableMods(Fl_Widget* widget, void* userdata);
	static void enableMods(Fl_Widget* widget, void* userdata);
	static void disableMods(Fl_Widget* widget, void* userdata);
	static void moveUpMods(Fl_Widget* widget, void* userdata);
	static void moveDownMods(Fl_Widget* widget, void* userdata);
	static void selectGameVersion(Fl_Widget* widget, void* userdata);
	static void maintainSubProcessState(void*);
};

#endif //JA2_LAUNCHER_H_H
