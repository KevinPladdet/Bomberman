#pragma once

class Game;
class Map;

enum class EnemyType
{
	BALLOON,
	ONION
};

class Enemy
{
public:
	Enemy(Game* m_game, Map* m_map, int2 _pos, EnemyType _type, int startFrame);
	void EnemyAnim(int startFrameLeft, int startFrameRight);
	void EnemyMove(float deltaTime);
	void SetRandomDirection();
	void EnemyDeath(int startFrame, int endFrame);
	void Shuffle(int2 array[4]);
	
	int2 pos;
	int frameCount = 0;

	// Enemy Death
	bool startDeathAnim = false;
	bool readyToDelete = false;

	EnemyType type;

private:
	// The enemy is 48x48, so add 24 to X and Y to get the middle
	static constexpr int enemySizeOffset = 24;

	// Enemy Animation
	Timer animTimer;
	static constexpr float timePerFrame = 0.10f;

	// Enemy Movement
	float moveSpeed = 0.1f;
	int2 direction = int2(0, 0); // Random movement direction
	Timer waitForTurnTimer;
	float waitForTime = 4.0f;

	// Enemy Death
	static constexpr float timePerFrameDeathAnim = 0.4f;
	int killScoreAmount = 100;

	Game* m_game = nullptr;
	Map* m_map = nullptr;
};