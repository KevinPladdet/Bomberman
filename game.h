// Template, 2024 IGAD Edition
// Get the latest version from: https://github.com/jbikker/tmpl8
// IGAD/NHTV/BUAS/UU - Jacco Bikker - 2006-2024

#pragma once

class AudioManager;
class TextManager;
class Menu;
class Map;
class Bomb;
class Player;

class Game : public TheApp
{
public:
	// game flow methods
	void Init();
	void Tick( float deltaTime );
	void Shutdown();
	// input handling
	void MouseUp( int ) { /* implement if you want to detect mouse button presses */ }
	void MouseDown( int ) { /* implement if you want to detect mouse button presses */ }
	void MouseMove( int x, int y ) { mousePos.x = x, mousePos.y = y; }
	void MouseWheel( float ) { /* implement if you want to handle the mouse wheel */ }
	void KeyUp(int key);
	void KeyDown(int key);
	// data members
	int2 mousePos = NULL;

	void SpawnBomb(float x, float y, bool isPlayerTwo);
	bool CheckPlayerBombCollision(Player* player, bool isPlayerTwo);

	void Singleplayer(float deltaTime);
	void Multiplayer(float deltaTime);
	void NewGame();
	void NewLife();
	void DeleteForSingleplayer();

	// Score Variables
	int score = 0;
	int topScore = 0;
	// Game Loop Variables
	int lives = 3;
	int stage = 1;
	int timeLeft = 200; // Timer of level (counts down to 0 and then enemies will spawn)
	bool deathSoundPlaying = false;

	bool singlePlayer = false;
	bool wasPlayingSingleplayer = false;
	bool wasPlayingMultiplayer = false;

private:
	AudioManager* m_am = nullptr;
	TextManager* m_textmanager = nullptr;
	Menu* m_menu = nullptr;
	Map* m_map = nullptr;
	Player* m_player = nullptr;
	Player* m_playerTwo = nullptr;

	static constexpr int MAX_BOMBS = 20; // Max amount of bombs to ever be placed at once by both players combined
	Bomb* m_bombs[MAX_BOMBS] = { nullptr };

	// Amount of bombs active on map for each player
	int p1BombCount = 0;
	int p2BombCount = 0;
};