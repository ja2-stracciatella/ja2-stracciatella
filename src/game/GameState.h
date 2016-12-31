#pragma once

enum GameMode
{
	GAME_MODE_GAME,
	GAME_MODE_EDITOR,
	GAME_MODE_EDITOR_AUTO,
	GAME_MODE_END
};


/** Global game state (singleton). */
class GameState
{
public:

  /** Get instance of the object. */
  static GameState* getInstance();

  /** Get current game mode. */
  GameMode getMode();

  /** Set editor mode. */
  void setEditorMode(bool autoLoad);
  void setDebugging(bool enabled);

  /** Check if we are in the editor mode. */
  bool isEditorMode();
  bool debugging();

private:

  GameMode m_mode;
  bool debug;

  /** Private constructor to avoid instantiation. */
  GameState();

  GameState(GameState const&);
  void operator=(GameState const&);
};
