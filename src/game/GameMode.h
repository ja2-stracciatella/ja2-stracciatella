#pragma once

enum GameRunMode
{
	GAME_MODE_GAME,
	GAME_MODE_EDITOR,
	GAME_MODE_EDITOR_AUTO,
	GAME_MODE_END
};


/** Singleton data object about how the game was run. */
class GameMode
{
public:

	/** Get instance of the object. */
	static GameMode* getInstance();

	/** Get current game mode. */
	GameRunMode getMode();

	/** Set editor mode. */
	void setEditorMode(bool autoLoad);
	void setDebugging(bool enabled);

	/** Check if we are in the editor mode. */
	bool isEditorMode();
	bool debugging();

private:

	GameRunMode m_mode;
	bool debug;

	/** Private constructor to avoid instantiation. */
	GameMode();

	GameMode(GameMode const&);
	void operator=(GameMode const&);
};
