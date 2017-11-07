#include "GameState.h"

/** Get instance of the object. */
GameState* GameState::getInstance()
{
	static GameState instance;
	return &instance;
}


/** Get current game mode. */
GameMode GameState::getMode()
{
	return m_mode;
}


/** Check if we are in the editor mode. */
bool GameState::isEditorMode()
{
	return (m_mode == GAME_MODE_EDITOR) || (m_mode == GAME_MODE_EDITOR_AUTO);
}

bool GameState::debugging()
{
	return debug;
}

/** Set editor mode. */
void GameState::setEditorMode(bool autoLoad)
{
	m_mode = autoLoad ? GAME_MODE_EDITOR_AUTO : GAME_MODE_EDITOR;
}

void GameState::setDebugging(bool enabled) {
	debug = enabled;
}


/** Private constructor to avoid instantiation. */
GameState::GameState()
	:m_mode(GAME_MODE_GAME)
{
	debug = false;
}
