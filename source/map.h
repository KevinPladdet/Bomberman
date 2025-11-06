#pragma once

class Game;
class AudioManager;
class Player;
class Enemy;
struct Tile;

enum class PowerUpType
{
	EXPLOSION_RANGE,
	EXTRA_BOMB
};

class Map
{
public:
	Map(Game* m_game, AudioManager* m_am);
	~Map();
	void GenerateBreakableBlocks();
	void SetupCollisions();
	void DrawMap(bool playerOne);
	void Splitscreen();
	void DrawPlayers(Player* m_player, Player* m_playerTwo);
	void UpdateCamera(float& playerX, bool isPlayerTwo, float playerSpeed, float deltaTime);
	void DestroyBreakableBlock(int tileX, int tileY);
	void SpawnPowerUp();
	void Enemies(float deltaTime);
	bool EnemyAABB(float2 playerPos, float2 playerSize, float2 enemyPos, float2 enemySize);
	void GenerateEnemies();
	void NextLevelDoor();
	void DrawDebugBox(int2 pos, int2 size, uint color);
	void DrawDebugBox(int posX, int posY, int sizeX, int sizeY, uint color);

	Surface* surface; // Main surface (UI is also drawn on here)
	Surface* surfaceLeft;
	Surface* surfaceRight;
	Surface* tempSurface; // is used in CreateMap to set either to surfaceLeft or surfaceRight

	// used for splitscreen and updating the camera
	float offset = 0;
	float offsetLeft = 0;
	float offsetRight = 0;

	static constexpr int mapTiles = 31; // amount of tiles mapLayout has
	static constexpr int tileSize = 48; // pixel size per sprite

	// used in for loop to check through tilePos
	int wallIndex = 0;
	
	static constexpr int SPAWN_BREAKABLE_BLOCKS = 44;
	static constexpr int TILE_AMOUNT = (13*32) + SPAWN_BREAKABLE_BLOCKS;
	int activeBreakableBlocks = 0; // increments every time a "-" is replaced by "B"
	Tile* tilePos[TILE_AMOUNT] = { nullptr };

	char mapLayout[13][32] =
	{
		"###############################",
		"#SSB--------------------------#",
		"#S#-#-#-#-#-#-#-#-#-#-#-#-#-#-#",
		"#B----------------------------#",
		"#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#",
		"#-----------------------------#",
		"#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#",
		"#-----------------------------#",
		"#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#",
		"#----------------------------B#",
		"#-#-#-#-#-#-#-#-#-#-#-#-#-#-#S#",
		"#--------------------------BSS#",
		"###############################"
	};

	// Enemies
	int spawn_balloon = 5;
	int spawn_onion = 0;
	int activeEnemies = 0;
	int enemyIndex = 0;
	static constexpr int ENEMY_AMOUNT = (13*32);
	Enemy* enemies[ENEMY_AMOUNT] = { nullptr };

	// PowerUp
	int powerUpIndex = 0;
	static constexpr int POWERUPSAMOUNT = 13 * 32;
	Tile* powerUps[POWERUPSAMOUNT] = { nullptr };
	
	// Bomb Variables
	int explosionRange = 1;
	int bombAmount = 1;
	static constexpr int BOMB_AMOUNT = 13 * 32;
	int2 bombPositions[BOMB_AMOUNT];

	// Game Over
	bool gameOverActivated = false;
	bool stageTransitionActivated = false;

	// Next Level Door
	bool doorSpawned = false;
	int doorX = 0;
	int doorY = 0;

	bool enable_debugging = false;

private:
	float playerSize = 42; // Spritesheet is 48x48 but for better hitboxes its 42

	// PowerUp
	int powerUpX;
	int powerUpY;

	Sprite* tileSetSprite = nullptr;
	Sprite* enemiesSprite = nullptr;

	Game* m_game = nullptr;
	AudioManager* m_am = nullptr;
};