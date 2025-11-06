#include "precomp.h"
#include "player.h"
#include "map.h"
#include "game.h"
#include "audiomanager.h"
#include "enemy.h"
#include "tile.h"
#include <iostream>

Timer Player::sfxTimer;

Player::Player(Map* m_map, Game* m_game, AudioManager* m_am)
	: m_map(m_map),
	  m_game(m_game),
	  m_am(m_am)
{
	playerSprite = new Sprite(new Surface("assets/Bomberman_Spritesheet_48.png"), 20);
}

Player::~Player()
{
	delete playerSprite;

	playerSprite = nullptr;
}

void Player::Update(float deltaTime)
{
#ifdef _DEBUG
	// Toggle to enable_debug (if enabled will have Box drawn for multiple hitboxes)
	if (GetAsyncKeyState('P') & 0x1001)
	{
		if (m_map->enable_debugging)
		{
			std::cout << "\nDisabled Debugging Hitboxes" << "\n";
			m_map->enable_debugging = false;
		}
		else
		{
			std::cout << "\nEnabled Debugging Hitboxes" << "\n";
			m_map->enable_debugging = true;
		}
	}
#endif

	if (!playerHit && !m_game->deathSoundPlaying)
	{
		LoopEnemyCollision();

		static bool bombKeyWasDown = false;
		bool bombKeyDown = !isPlayerTwo && GetAsyncKeyState('X') & 0x08000 ||
							isPlayerTwo && GetAsyncKeyState(VK_SPACE) & 0x08000;
		if (bombKeyDown && !bombKeyWasDown)
		{
			int playerTileX = static_cast<int>((playerX + playerSizeOffset) / m_map->tileSize);
			int playerTileY = static_cast<int>((playerY + playerSizeOffset - UI_OFFSET) / m_map->tileSize);
			bool canPlaceBomb = true;

			for (int i = 0; i < m_map->BOMB_AMOUNT; ++i)
			{
				if (m_map->bombPositions[i].x == playerTileX && m_map->bombPositions[i].y == playerTileY)
				{
					canPlaceBomb = false;
				}
			}
			if (canPlaceBomb && bombPlaceCooldownTimer.elapsed() > bombPlaceCooldown)
			{
				bombPlaceCooldownTimer.reset();
				m_game->SpawnBomb(playerX, playerY, isPlayerTwo);
			}
		}
		bombKeyWasDown = bombKeyDown;

		bool moving = false;

		// UP
		if (!isPlayerTwo && GetAsyncKeyState('W') & 0x08000 ||
			isPlayerTwo && GetAsyncKeyState(VK_UP) & 0x08000)
		{
			MoveAnim(UP);
			if (!snappingAlreadyActive)
			{
				PlayerSnapping(UP);
			}
			LoopTileCollision(playerY, -playerSpeed * deltaTime);
			moving = true;
		}

		// DOWN
		if (!isPlayerTwo && GetAsyncKeyState('S') & 0x08000 ||
			isPlayerTwo && GetAsyncKeyState(VK_DOWN) & 0x08000)
		{
			MoveAnim(DOWN);
			if (!snappingAlreadyActive)
			{
				PlayerSnapping(DOWN);
			}
			LoopTileCollision(playerY, +playerSpeed * deltaTime);
			moving = true;
		}

		// LEFT
		if (!isPlayerTwo && GetAsyncKeyState('A') & 0x08000 ||
			isPlayerTwo && GetAsyncKeyState(VK_LEFT) & 0x08000)
		{
			MoveAnim(LEFT);
			if (!snappingAlreadyActive)
			{
				PlayerSnapping(LEFT);
			}
			LoopTileCollision(playerX, -playerSpeed * deltaTime);
			moving = true;
		}

		// RIGHT
		if (!isPlayerTwo && GetAsyncKeyState('D') & 0x08000 ||
			isPlayerTwo && GetAsyncKeyState(VK_RIGHT) & 0x08000)
		{
			MoveAnim(RIGHT);
			if (!snappingAlreadyActive)
			{
				PlayerSnapping(RIGHT);
			}
			LoopTileCollision(playerX, +playerSpeed * deltaTime);
			moving = true;
		}

		// Reset snappingAlreadyActive if no movement keys are pressed
		if (!moving)
		{
			snappingAlreadyActive = false;
		}

		m_map->UpdateCamera(playerX, isPlayerTwo, playerSpeed, deltaTime);
	}
	
	if (playerHit && !deathAnimFinished)
	{
		PlayerDeath();
	}
}

void Player::MoveAnim(PlayerDirection direction)
{	
	if (sfxTimer.elapsed() > sfxTimePerFrame)
	{
		if (direction == LEFT || direction == RIGHT)
		{
			m_am->PlaySFX("sfx/Walk_Horizontal_SFX.mp3", 1.0f);
		}
		else
		{
			m_am->PlaySFX("sfx/Walk_Vertical_SFX.mp3", 2.0f);
		}
		sfxTimer.reset();
	}

	if (animTimer.elapsed() > timePerFrame)
	{
		animIndex++;
		animIndex = animIndex % 3;
		animTimer.reset();
	}

	switch (direction)
	{
		case LEFT: 
			frameCount = 0; // Left Move Animation
			break;
		case DOWN: 
			frameCount = 3; // Down Move Animation
			break;
		case RIGHT: 
			frameCount = 6; // Right Move Animation
			break;
		case UP: 
			frameCount = 9; // Up Move Animation
			break;
	}
	
	playerSprite->SetFrame(frameCount + animIndex);
}

void Player::PlayerTwo()
{
	isPlayerTwo = true;
	playerX = static_cast<int>((m_map->mapTiles * m_map->tileSize) - m_map->tileSize * 2); // Puts the player 2 tiles away from the full right
	playerY = SCRHEIGHT - UI_OFFSET;
	
	// Start the camera at the far right for playerTwo
	m_map->offsetRight = (m_map->mapTiles * m_map->tileSize) - (SCRWIDTH / 2);
}

// I wanted to implement this function at first, but I eventually decided not to use it
void Player::ProcessInput(int key, bool keyDown)
{
	/*if (key == GLFW_KEY_0) 
	{
		if (keyDown)
		{
			std::cout << "Pressed 0 Key" << endl;
		}
	}*/
}

bool Player::CheckCollisionAABB(Tile* tile)
{
	return (playerX < tile->pos.x + m_map->tileSize &&
			playerX + playerSize.x > tile->pos.x &&
			playerY < tile->pos.y + m_map->tileSize &&
			playerY + playerSize.y > tile->pos.y);
}

void Player::LoopTileCollision(float& pos, float speed)
{
	pos += speed;

	// Tile Collision
	for (int i = 0; i < m_map->wallIndex; ++i)
	{
		if (CheckCollisionAABB(m_map->tilePos[i]))
		{
			pos -= speed;
			break;
		}
	}

	// Bomb Collision
	if (m_game->CheckPlayerBombCollision(this, isPlayerTwo))
	{
		pos -= speed;
		return;
	}

	// PowerUp Collision
	for (int i = 0; i < m_map->powerUpIndex; ++i)
	{
		if (CheckCollisionAABB(m_map->powerUps[i]))
		{
			m_am->PlaySFX("sfx/Pickup_Powerup_SFX.mp3", 1.0f);
			if (m_map->powerUps[i]->powerUpType == PowerUpType::EXPLOSION_RANGE)
			{
				std::cout << "Player picked up EXPLOSION_RANGE powerup!" << "\n";
				m_map->explosionRange++;
			}
			if (m_map->powerUps[i]->powerUpType == PowerUpType::EXTRA_BOMB)
			{
				std::cout << "Player picked up EXTRA_BOMB powerup!" << "\n";
				m_map->bombAmount++;
			}
			
			delete m_map->powerUps[i];
			// Clear up gap in powerUps because of deleting it
			for (int j = i; j < m_map->powerUpIndex - 1; ++j)
			{
				m_map->powerUps[j] = m_map->powerUps[j + 1];
			}
			m_map->powerUpIndex--;
		}
	}

	// Door Collision
	if (playerX < m_map->doorX + m_map->tileSize &&
		playerX + playerSize.x >  m_map->doorX &&
		playerY <  m_map->doorY + UI_OFFSET + m_map->tileSize &&
		playerY + playerSize.y >  m_map->doorY + UI_OFFSET
		&& m_map->activeEnemies <= 0)
	{
		EnterDoor();
	}
}

void Player::LoopEnemyCollision()
{
	float2 playerPos = { playerX, playerY };
	float2 playerSize = { 42, 42 };
	float2 enemySize = { 42, 42 };

	// Enemy Collision
	for (int i = 0; i < m_map->enemyIndex; ++i)
	{
		Enemy* enemy = m_map->enemies[i];

		float2 enemyPos = { static_cast<float>(m_map->enemies[i]->pos.x), static_cast<float>(m_map->enemies[i]->pos.y) };

		// AABB collision check with player and enemies
		if (m_map->EnemyAABB(playerPos, playerSize, enemyPos, enemySize) && !playerHit && !enemy->startDeathAnim)
		{
			CallOnceOnHit();
		}
	}
}

void Player::PlayerDeath()
{
	int firstFrameOfDeathAnim = 14;
	int lastFrameOfDeathAnim = 19;
	float timePerFrameDeathAnim = 0.4;

	// Make sure it starts at the first frame of the death animation
	if (frameCount < firstFrameOfDeathAnim)
	{
		frameCount = firstFrameOfDeathAnim;
	}
	
	if (animTimer.elapsed() > timePerFrameDeathAnim && frameCount < lastFrameOfDeathAnim)
	{
		frameCount++;
		playerSprite->SetFrame(frameCount);
		animTimer.reset();
	}
	
	// Activate game over after timePerFrameDeathAnim / 0.4s
	if (animTimer.elapsed() > timePerFrameDeathAnim)
	{
		if (m_game->lives > 1)
		{
			m_game->lives--;
			printf("Lives remaining: %d\n", m_game->lives);
			deathAnimFinished = true;
			m_map->stageTransitionActivated = true;
		}
		else
		{
			deathAnimFinished = true;
			m_map->gameOverActivated = true;
		}
	}
}

void Player::CallOnceOnHit()
{
	playerHit = true;

	if (!m_game->deathSoundPlaying)
	{
		if (!isPlayerTwo)
		{
			printf("\nPlayer1 died\n\n");
		}
		else
		{
			printf("\nPlayer2 died\n\n");
		}

		m_am->PlaySFX("sfx/Death_SFX.mp3", 1.0f);
		m_game->deathSoundPlaying = true;
	}
}

void Player::EnterDoor()
{
	printf("\n- - - LEVEL COMPLETED - - -\n");
	m_game->stage++;
	m_game->lives = 3;
	m_map->stageTransitionActivated = true;
}

void Player::PlayerSnapping(PlayerDirection direction)
{
	snappingAlreadyActive = true;

	int playerTileX = static_cast<int>(playerX + playerSizeOffset);
	int playerTileY = static_cast<int>(playerY + playerSizeOffset - UI_OFFSET);

	// Convert player positions to grid positions
	int checkTileX = playerTileX / m_map->tileSize;
	int checkTileY = playerTileY / m_map->tileSize;

	switch (direction)
	{
	case LEFT:
		checkTileX -= 1; break;

	case RIGHT:
		checkTileX += 1; break;

	case UP:
		checkTileY -= 1; break;

	case DOWN:
		checkTileY += 1; break;
	}

	// Wall collision check
	if (m_map->mapLayout[checkTileY][checkTileX] == '#' ||
		m_map->mapLayout[checkTileY][checkTileX] == 'B')
	{
		return;
	}

	// Convert back to world positions instead of grid positions
	checkTileX = checkTileX * m_map->tileSize;
	checkTileY = checkTileY * m_map->tileSize;

	float distance = 0.0f;
	switch (direction)
	{
	case LEFT:
		distance = abs((playerY + playerSize.y / 2 - UI_OFFSET) - (checkTileY + m_map->tileSize / 2));
		break;
	case RIGHT:
		distance = abs((playerY + playerSize.y / 2 - UI_OFFSET) - (checkTileY + m_map->tileSize / 2));
		break;
	case UP:
		distance = abs((playerX + playerSize.x / 2) - (checkTileX + m_map->tileSize / 2));
		break;
	case DOWN:
		distance = abs((playerX + playerSize.x / 2) - (checkTileX + m_map->tileSize / 2));
		break;
	}

	// Snap when distance is close enough
	if (distance <= playerSizeOffset * 2 * 0.95f)
	{
		switch (direction)
		{
		case LEFT:
			playerY = checkTileY + (m_map->tileSize / 2) - (playerSize.y / 2) + UI_OFFSET;
			break;

		case RIGHT:
			playerY = checkTileY + (m_map->tileSize / 2) - (playerSize.y / 2) + UI_OFFSET;
			break;

		case UP:
			playerX = checkTileX + (m_map->tileSize / 2) - (playerSize.x / 2);
			break;

		case DOWN:
			playerX = checkTileX + (m_map->tileSize / 2) - (playerSize.x / 2);
			break;
		}
	}
}