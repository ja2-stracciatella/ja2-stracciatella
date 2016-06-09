You cannot at the moment:
  - change ammo_types.json
  - add more weapons
  - add more magazines


Settings in game.json
=====================

  map_hotkey_ctrl_i

        Enables Ctrl-i hotkey, which opens and closes sector inventory screen.

		Vanilla setting: FALSE
  ----------------------------------------

  tactical_hotkey_j

        Enables 'j' hotkey for climbing up/down.

		Vanilla setting: FALSE
  ----------------------------------------

  tactical_hotkey_shift_n

        Enables 'Shift-n' hotkey which switches the current squad's head gear to the night mode.

		Vanilla setting: FALSE
  ----------------------------------------

  tactical_hotkey_ctrl_n

        Enables 'Ctrl-n' hotkey which switches the current squad's head gear to the day mode.

		Vanilla setting: FALSE
  ----------------------------------------

  ms_per_game_cycle

        Number of milliseconds for one game cycle.
        25 ms gives approx. 40 cycles per second (and 40 frames per second, since the screen
        is updated on every cycle).

        Decreasing this value will increase speed of the game (enemy movement, bullets speed, etc).

		Stracciatella standard value: 25
  ----------------------------------------

  starting_cash_easy / starting_cash_medium / starting_cash_hard

        Amount of money the player starts the game with on each difficulty.

		Vanilla values: 45000/35000/30000 (easy/medium/hard).
  ----------------------------------------

  drop_everything

        Decides, if enemies drop all of their equipment when they die.

		Vanilla setting: FALSE
  ----------------------------------------

  all_drops_visible

		Decides, if dropped items, regardless of where and by whom they were dropped, are always visible to the player.

		Vanilla setting: FALSE
  ----------------------------------------

  enemy_weapon_minimal_status

		The minimal status in % for each gun in the hand of enemy soldiers. Note that this value has no influence on enemy behaviour (CtH, jams).

		Vanilla value: 0
  ----------------------------------------
