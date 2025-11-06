#pragma once

class Game;
class AudioManager;
class TextManager;

class Menu
{
public:
	enum GameState
	{
		MAIN_MENU,
		SINGLEPLAYER,
		MULTIPLAYER,
		GAME_OVER,
		STAGE_TRANSITION,
		EXIT
	};

	Menu(Game* m_game, AudioManager* m_am, TextManager* m_textmanager);
	~Menu();
	void InMainMenu(Surface* surface);
	void DrawMenu(Surface* surface);
	void DrawGameOverMenu(Surface* surface);
	void DrawStageTransitionMenu(Surface* surface);
	void PlaySelectSFX();

	GameState game_state;
	GameState selected_state;

	// Timers
	Timer selectSfxCooldownTimer;
	Timer endOfMusicTimer;

	int selected = 2; // Sets default selected to Multiplayer (which is 2)

private:
	int totalAmountOptions = 3; // Same value as the total amount of selection options

	Sprite* mainMenu_Singleplayer_Sprite = nullptr;
	Sprite* mainMenu_Multiplayer_Sprite = nullptr;
	Sprite* mainMenu_Exit_Sprite = nullptr;

	Game* m_game = nullptr;
	AudioManager* m_am = nullptr;
	TextManager* m_textmanager = nullptr;
};