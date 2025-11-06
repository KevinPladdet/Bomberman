#pragma once

class Map;
class Game;
class AudioManager;
struct Tile;

class Player
{
public:

	enum PlayerDirection
	{
		UP,
		DOWN,
		LEFT,
		RIGHT
	};

	Player(Map* m_map, Game* m_game, AudioManager* m_am);
	~Player();
	void Update(float deltaTime);
	void MoveAnim(PlayerDirection direction);
	void PlayerTwo();
	void ProcessInput(int key, bool keyDown);
	bool CheckCollisionAABB(Tile* tile);
	void LoopTileCollision(float& pos, float speed);
	void LoopEnemyCollision();
	void PlayerDeath();
	void CallOnceOnHit();
	void EnterDoor();
	void PlayerSnapping(PlayerDirection direction);

	Sprite* playerSprite = nullptr;

	float playerSpeed = 0.2;
	float playerX = 50;
	float playerY = 144; // 144 is the distance to put player in the top left of tilemap
	float2 playerSize = { 42, 42 }; // Bomberman spritesheet is 48px but for better hitboxes I changed this to 42px

	bool isPlayerTwo = false;

	// Timer for walking SFX
	static Timer sfxTimer;

private:
	bool playerHit = false;

	static constexpr int playerSizeOffset = 24;
	bool snappingAlreadyActive = false;

	// Anim
	Timer animTimer;
	static constexpr float timePerFrame = 0.2; // time in between animation frames
	int animIndex = 0;
	int frameCount = 0;
	bool deathAnimFinished = false;

	// Timer for bomb place cooldown
	Timer bombPlaceCooldownTimer;
	float bombPlaceCooldown = 0.15;

	// SFX
	static constexpr float sfxTimePerFrame = 0.4;

	Map* m_map = nullptr;
	Game* m_game = nullptr;
	AudioManager* m_am = nullptr;
};