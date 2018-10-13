#ifndef JA2_LAUNCHER_H_H
#define JA2_LAUNCHER_H_H

#include "StracciatellaLauncher.h"
#include "RustInterface.h"

class Launcher : public StracciatellaLauncher {
public:
	Launcher(std::string exePath, engine_options_t* engine_options);

	void show();
	void initializeInputsFromDefaults();
	int writeJsonFile();
private:
	std::string exePath;
	engine_options_t* engine_options;

	void populateChoices();
	void enablePredefinedResolutions();
	void enableCustomResolutions();
	void startExecutable(bool asEditor);
	static void openDataDirectorySelector(Fl_Widget *btn, void *userdata);
	static void enablePredefinedResolutionSelection(Fl_Widget* btn, void* userdata);
	static void enableCustomResolutionSelection(Fl_Widget* btn, void* userdata);
	static void startGame(Fl_Widget* btn, void* userdata);
	static void startEditor(Fl_Widget* btn, void* userdata);
};

#endif //JA2_LAUNCHER_H_H
