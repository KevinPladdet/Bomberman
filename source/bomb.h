#pragma once
#include <array>

class Game;
class Map;
class AudioManager;
class Player;
class Enemy;

class Bomb
{
public:
	Bomb(Game* m_game, Map* m_map, AudioManager* m_am, Player* m_player, Player* m_playerTwo, float x, float y);
	~Bomb();
	void PlaceBomb(float playerX, float playerY);
	void Update();
	void BombAnim();
	bool AABB(float2 playerPos, float2 playerSize, float2 bombPos, float2 bombSize);
	bool CheckBombCollision(Player* m_collidingPlayer, bool isPlayerTwo);
	void Explode(float2 bombPos);
	void ExplodeInUpdate(float2 bombPos);
	void BombBlastAnim();
	void BreakableBlockAnim(int tileX, int tileY);
	void BombBlastDirection(char direction, bool endRange);

	bool IsDone() const { return isDone; }

	bool ownerIsPlayerTwo = false;
	bool exitedByPlayer[2] = { false, false };

private:
	float2 bombPos;
	bool isDone = false;

	bool bombPlaced = false;
	bool bombCollision = false;

	// The player is 48x48, so add 24 to X and Y to put bomb in middle
	static constexpr int playerSizeOffset = 24;
	float2 bombSize = { 48, 48 };

	// Bomb ticking animation
	Timer animTimer;
	static constexpr float timePerFrame = 0.3; // time in between animation frames
	int animIndex = 0;
	std::array<int, 9> frameSequence = {2, 1, 0, 1, 2, 1, 0, 1, 2};
	
	// Bomb blast animation
	Timer blastAnimTimer;
	bool blastCompleted = true;
	float blastDurationAnim = 0.6f;

	// Breakable blocks animation
	Timer bbAnimTimer;
	static constexpr float bbTimePerFrame = 0.1;
	int bbAnimIndex = 0;
	int bbFrameCount = 3;

	Sprite* bombSprite = nullptr;
	Sprite* bombBlastSprite = nullptr;
	Sprite* breakableBlockSprite = nullptr;

	Game* m_game = nullptr;
	Map* m_map = nullptr;
	AudioManager* m_am = nullptr;
	Player* m_player = nullptr;
	Player* m_playerTwo = nullptr;
};