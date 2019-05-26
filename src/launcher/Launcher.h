#ifndef JA2_LAUNCHER_H_H
#define JA2_LAUNCHER_H_H

#include "StracciatellaLauncher.h"
#include "RustInterface.h"

class Launcher : public StracciatellaLauncher {
public:
	Launcher(std::string exePath, EngineOptions* engine_options);

	void show();
	void initializeInputsFromDefaults();
	int writeJsonFile();
private:
	std::string exePath;
	EngineOptions* engine_options;

	void populateChoices();
	void startExecutable(bool asEditor);
	bool resolutionIsInvalid();
	static void openDataDirectorySelector(Fl_Widget *btn, void *userdata);
	static void startGame(Fl_Widget* btn, void* userdata);
	static void startEditor(Fl_Widget* btn, void* userdata);
	static void guessVersion(Fl_Widget* btn, void* userdata);
	static void setPredefinedResolution(Fl_Widget* btn, void* userdata);
	static void inspectResolution(Fl_Widget* btn, void* userdata);
};

#endif //JA2_LAUNCHER_H_H
