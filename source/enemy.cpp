#include "precomp.h"
#include "enemy.h"
#include "game.h"
#include "map.h"

Enemy::Enemy(Game* m_game, Map* m_map, int2 _pos, EnemyType _type, int startFrame)
	:
	m_game(m_game),
	m_map(m_map),
	pos(_pos),
	type(_type),
	frameCount(startFrame)
{
	if (type == EnemyType::ONION)
	{
		killScoreAmount = 200;
		waitForTime = 3.0f;
	}

	SetRandomDirection();
}

void Enemy::EnemyAnim(int startFrameLeft, int startFrameRight)
{
	// Left
	if (direction.x == -1 || direction.y == -1)
	{
		if (animTimer.elapsed() > timePerFrame)
		{
			frameCount++;
			animTimer.reset();
			if (frameCount >= startFrameLeft + 2) // + 2 because each direction has 3 frames in total
			{
				frameCount = startFrameLeft;
			}
		}
	}

	// Right
	if (direction.x == 1 || direction.y == 1)
	{
		if (animTimer.elapsed() > timePerFrame)
		{
			frameCount++;
			animTimer.reset();
			if (frameCount >= startFrameRight + 2) // + 2 because each direction has 3 frames in total
			{
				frameCount = startFrameRight;
			}
		}
	}
}

void Enemy::EnemyMove(float deltaTime)
{
    int tileX = ((pos.x + enemySizeOffset) / m_map->tileSize);
    int tileY = ((pos.y + enemySizeOffset - UI_OFFSET) / m_map->tileSize);

	if (direction.x == 1)
	{
		tileX = (pos.x + 2 * enemySizeOffset) / m_map->tileSize;
	}
	else if (direction.x == -1)
	{
		tileX = (pos.x) / m_map->tileSize;
	}
	else if (direction.y == 1)
	{
		tileY = (pos.y + 2 * enemySizeOffset - UI_OFFSET) / m_map->tileSize;
	}
	else if (direction.y == -1)
	{
		tileY = (pos.y - UI_OFFSET) / m_map->tileSize;
	}

	// Go the other way when hitting a wall or block
	if (m_map->mapLayout[tileY][tileX] == '#' ||
		m_map->mapLayout[tileY][tileX] == 'B')
	{
		direction.x *= -1;
		direction.y *= -1;
	}
	// Go the other way when hitting a bomb
	for (int i = 0; i < m_map->BOMB_AMOUNT; ++i)
	{
		if (m_map->bombPositions[i].x == tileX && m_map->bombPositions[i].y == tileY)
		{
 			direction.x *= -1;
			direction.y *= -1;
		}
	}

	//All 4 directions for the bomb to check tiles for any breakable blocks
	int directions[4][2] =
	{
		{ -1, 0 }, // Left
		{ 1, 0 },  // Right
		{ 0, -1 }, // Up
		{ 0, 1 }   // Down
	};

	int centerX = ((pos.x + enemySizeOffset) / m_map->tileSize);
	int centerY = ((pos.y + enemySizeOffset - UI_OFFSET) / m_map->tileSize);

	for (int i = 0; i < 4; i++)
	{
		int dirX = directions[i][0];
		int dirY = directions[i][1];

		// Don't check tiles for '-' of current axis direction
		if (direction.x != 0 && dirX != 0)
		{
			continue;
		}
		if (direction.y != 0 && dirY != 0)
		{
			continue;
		}

		int checkX = centerX + dirX;
		int checkY = centerY + dirY;

		int2 tileCenter = { centerX * m_map->tileSize + enemySizeOffset, centerY * m_map->tileSize + enemySizeOffset + UI_OFFSET };
		int2 enemyCenter = { pos.x + enemySizeOffset, pos.y + enemySizeOffset };
		float distance = length(enemyCenter - tileCenter);

		if (m_map->mapLayout[checkY][checkX] == '-' ||
			m_map->mapLayout[checkY][checkX] == 'S')
		{
			int random = rand() % 10;
			if (random == 1 && waitForTurnTimer.elapsed() > waitForTime
				&& distance <= 1)
			{
				waitForTurnTimer.reset();
				direction.x = dirX;
				direction.y = dirY;
			}
			break;
		}
	}

	if (type == EnemyType::BALLOON)
	{
		moveSpeed = 0.1f;
	}
	if (type == EnemyType::ONION)
	{
		moveSpeed = 0.15;
	}

	pos.x += static_cast<int>(direction.x * moveSpeed * deltaTime);
	pos.y += static_cast<int>(direction.y * moveSpeed * deltaTime);
}

void Enemy::SetRandomDirection()
{
	// Randomize starting direction
	int2 directions[4] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
	Shuffle(directions);

	int tileX = ((pos.x + enemySizeOffset) / m_map->tileSize);
	int tileY = ((pos.y + enemySizeOffset - UI_OFFSET) / m_map->tileSize);

	if (direction.x == 1)
	{
		tileX = (pos.x + 2 * enemySizeOffset) / m_map->tileSize;
	}
	else if (direction.x == -1)
	{
		tileX = (pos.x) / m_map->tileSize;
	}
	else if (direction.y == 1)
	{
		tileY = (pos.y + 2 * enemySizeOffset - UI_OFFSET) / m_map->tileSize;
	}
	else if (direction.y == -1)
	{
		tileY = (pos.y - UI_OFFSET) / m_map->tileSize;
	}
	
	for (int i = 0; i < 4; i++)
	{
		int dirX = directions[i].x;
		int dirY = directions[i].y;

		int checkX = tileX + dirX;
		int checkY = tileY + dirY;

		direction = directions[i];

		if (m_map->mapLayout[checkY][checkX] != '#' &&
			m_map->mapLayout[checkY][checkX] != 'B')
		{
			break;
		}
	}
}

void Enemy::EnemyDeath(int startFrame, int endFrame)
{
	// Make sure it starts at the first frame of the death animation
	if (frameCount < startFrame)
	{
		frameCount = startFrame;
	}

	if (animTimer.elapsed() > timePerFrameDeathAnim)
	{
		frameCount++;
		animTimer.reset();
		if (frameCount > endFrame)
		{
			m_game->score += killScoreAmount; // +100 to score
			m_map->activeEnemies--;
			readyToDelete = true; // Enemy will now be deleted
		}
	}
}


void Enemy::Shuffle(int2 array[4])
{
	for (int i = 0; i < 4 - 1; i++)
	{
		int j = i + rand() / (RAND_MAX / (4 - i) + 1);
		int2 t = array[j];
		array[j] = array[i];
		array[i] = t;
	}
}