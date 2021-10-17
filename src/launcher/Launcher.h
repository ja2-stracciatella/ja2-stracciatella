#ifndef JA2_LAUNCHER_H_H
#define JA2_LAUNCHER_H_H

#include "StracciatellaLauncher.h"
#include "RustInterface.h"

#include <string_theory/string>

#include <iostream>
#include <iterator>
#include <set>

struct sortMods {
    bool operator() (ST::string a, ST::string b) const {
        return a.compare(b) < 0;
    }
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
	RustPointer<EngineOptions> engine_options;
	std::set<ST::string, sortMods> availableMods;

	void populateChoices();
	void startExecutable(bool asEditor);
	bool resolutionIsInvalid();
	void update(bool changed, Fl_Widget *widget);
	static void openGameDirectorySelector(Fl_Widget *btn, void *userdata);
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
};

#endif //JA2_LAUNCHER_H_H
