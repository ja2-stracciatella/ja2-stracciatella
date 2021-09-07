#include "GameMode.h"

/** Get instance of the object. */
GameMode* GameMode::getInstance()
{
	static GameMode instance;
	return &instance;
}


/** Get current game mode. */
GameRunMode GameMode::getMode()
{
	return m_mode;
}


/** Check if we are in the editor mode. */
bool GameMode::isEditorMode()
{
	return (m_mode == GAME_MODE_EDITOR) || (m_mode == GAME_MODE_EDITOR_AUTO);
}

bool GameMode::debugging()
{
	return debug;
}

/** Set editor mode. */
void GameMode::setEditorMode(bool autoLoad)
{
	m_mode = autoLoad ? GAME_MODE_EDITOR_AUTO : GAME_MODE_EDITOR;
}

void GameMode::setDebugging(bool enabled) {
	debug = enabled;
}


/** Private constructor to avoid instantiation. */
GameMode::GameMode()
	:m_mode(GAME_MODE_GAME)
{
	debug = false;
}
