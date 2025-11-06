#include "precomp.h"
#include "game.h"
#include "map.h"
#include "player.h"
#include "enemy.h"
#include "audiomanager.h"
#include "tile.h"
#include <iostream>

Map::Map(Game* m_game, AudioManager* m_am)
	:
	m_game(m_game),
	m_am(m_am)
{
	tileSetSprite = new Sprite(new Surface("assets/Tileset_Spritesheet_48.png"), 24);
	enemiesSprite = new Sprite(new Surface("assets/Enemies_Spritesheet_48.png"), 40);

	surfaceLeft = new Surface(SCRWIDTH / 2, SCRHEIGHT);
	surfaceRight = new Surface(SCRWIDTH / 2, SCRHEIGHT);
	
#ifdef _DEBUG
	enable_debugging = true;
#endif

#ifdef _RELEASE
	enable_debugging = false;
#endif

	surfaceLeft->Clear(0xBDBEBD);
	surfaceRight->Clear(0xBDBEBD);

	GenerateBreakableBlocks();
	SetupCollisions();
	GenerateEnemies();

	for (int i = 0; i < BOMB_AMOUNT; ++i)
	{
		bombPositions[i] = { -1, -1 }; // -1 means "empty"
	}
}

Map::~Map()
{
	delete tileSetSprite;
	delete enemiesSprite;
	delete surfaceLeft;
	delete surfaceRight;

	tileSetSprite = nullptr;
	enemiesSprite = nullptr;
	surfaceLeft = nullptr;
	surfaceRight = nullptr;

	//  Delete all tiles in tilePos (used for saving int2 of tiles with collisions)
	for (int i = 0; i < wallIndex; ++i)
	{
		delete tilePos[i];
		tilePos[i] = nullptr;
	}

	// Delete all powerUps collision values
	for (int i = 0; i < powerUpIndex; i++)
	{
		delete powerUps[i];
		powerUps[i] = nullptr;
	}

	// Delete all enemy collision values
	for (int i = 0; i < enemyIndex; i++)
	{
		delete enemies[i];
		enemies[i] = nullptr;
	}
}

void Map::GenerateBreakableBlocks()
{ // Using the while loop with rand() is not very efficient, will try to make it more efficient eventually
	while (activeBreakableBlocks < SPAWN_BREAKABLE_BLOCKS)
	{
		for (int x = 0; x < 31; ++x)
		{
			for (int y = 0; y < 13; ++y)
			{
				int random = rand() % 100;
				if (mapLayout[y][x] == '-' && activeBreakableBlocks < SPAWN_BREAKABLE_BLOCKS && random == 0)
				{
					mapLayout[y][x] = 'B';
					activeBreakableBlocks++;
				}
			}
		}
	}
}

void Map::SetupCollisions()
{
	for (int x = 0; x < 31; ++x)
	{
		for (int y = 0; y < 13; ++y)
		{
			if (mapLayout[y][x] == '#' || mapLayout[y][x] == 'B')
			{
				Tile* tile = new Tile(int2((tileSize * x), ((tileSize * y) + UI_OFFSET)), false);
				tilePos[wallIndex] = tile;
				wallIndex++;
			}
		}
	}
}

void Map::DrawMap(bool playerOne)
{
	// Set surface and offset based on if its player or playerTwo
	if (m_game->singlePlayer)
	{
		tempSurface = surface;
	}
	else if (playerOne)
	{
		tempSurface = surfaceLeft;
		offset = offsetLeft;
	}
	else
	{
		tempSurface = surfaceRight;
		offset = offsetRight;
	}

	for (int x = 0; x < 31; ++x)
	{
		for (int y = 0; y < 13; ++y)
		{
			switch (mapLayout[y][x])
			{
			case '#': tileSetSprite->SetFrame(1); break;
			case 'B': tileSetSprite->SetFrame(2); break;
			case '-': tileSetSprite->SetFrame(0); break;
			case 'S': tileSetSprite->SetFrame(0); break; // Safe spot, breakable blocks cant spawn here
			default:
				std::cout << "No output for mapLayout char in DrawMap()\n";
				break;
			}
			tileSetSprite->Draw(tempSurface, static_cast<int>((tileSize * x) - offset), static_cast<int>((tileSize * y) + UI_OFFSET));
		}
	}

	// Draw PowerUps
	SpawnPowerUp();
}

void Map::Splitscreen()
{
	if (!m_game->singlePlayer)
	{
		surfaceLeft->CopyTo(surface, 0, 0);
		surfaceRight->CopyTo(surface, SCRWIDTH / 2, 0);
		DrawMap(0); // Map for playerTwo
	}
	DrawMap(1); // Map for player
}

void Map::DrawPlayers(Player* m_player, Player* m_playerTwo)
{
	if (!m_game->singlePlayer)
	{
		m_player->playerSprite->Draw(surfaceLeft, static_cast<int>(m_player->playerX - offsetLeft), static_cast<int>(m_player->playerY));
		m_player->playerSprite->Draw(surfaceRight, static_cast<int>(m_player->playerX - offsetRight), static_cast<int>(m_player->playerY));

		m_playerTwo->playerSprite->Draw(surfaceLeft, static_cast<int>(m_playerTwo->playerX - offsetLeft), static_cast<int>(m_playerTwo->playerY));
		m_playerTwo->playerSprite->Draw(surfaceRight, static_cast<int>(m_playerTwo->playerX - offsetRight), static_cast<int>(m_playerTwo->playerY));
	}
	else
	{
		m_player->playerSprite->Draw(surface, static_cast<int>(m_player->playerX - offset), static_cast<int>(m_player->playerY));
	}
	
	// Debugging hitboxes
	if (enable_debugging && !m_game->singlePlayer)
	{
		// Player 1
		DrawDebugBox
		(
			static_cast<int>(m_player->playerX + playerSize / 2),
			static_cast<int>(m_player->playerY + playerSize / 2 - UI_OFFSET),
			static_cast<int>(playerSize),
			static_cast<int>(playerSize),
			0x00FF00
		);
		// Player 2
		DrawDebugBox
		(
			static_cast<int>(m_playerTwo->playerX + playerSize / 2),
			static_cast<int>(m_playerTwo->playerY + playerSize / 2 - UI_OFFSET),
			static_cast<int>(playerSize),
			static_cast<int>(playerSize),
			0x00FF00
		);
	}
}

void Map::UpdateCamera(float& playerX, bool isPlayerTwo, float playerSpeed, float deltaTime)
{
	int totalMapWidth = mapTiles * tileSize;

	// Singleplayer camera
	if (m_game->singlePlayer)
	{
		const float halfScreen = SCRWIDTH * 0.5f;

		// Center camera on player
		offset = playerX - halfScreen;

		// Clamp to map bounds
		if (offset < 0)
		{
			offset = 0;
		}
		else if (offset > totalMapWidth - SCRWIDTH)
		{
			offset = static_cast<float>(totalMapWidth - SCRWIDTH);
		}
		return;
	}

	// Camera starts moving when player is 200px away from screen edge
	float boundary = 310.0f;

	const float rightBoundary = (SCRWIDTH / 2) - boundary;

	float& offset = !isPlayerTwo ? offsetLeft : offsetRight;

	// Only move map if player is at screen edge
	if (playerX > boundary && playerX < totalMapWidth - boundary)
	{
		float screenX = playerX - offset;

		if (screenX < boundary && offset > 0)
		{
			offset -= playerSpeed * deltaTime;
		}
		else if (screenX > rightBoundary)
		{
			offset += playerSpeed * deltaTime;
		}
	}
}

void Map::DestroyBreakableBlock(int tileX, int tileY)
{
	mapLayout[tileY][tileX] = '-'; // Change breakable block to floorTile
	activeBreakableBlocks--;

	tileX = tileX * tileSize;
	tileY = tileY * tileSize + UI_OFFSET;

	// Draw green floor to prevent breakable block appearing for a split second
	tileSetSprite->SetFrame(0);
	if (!m_game->singlePlayer)
	{
		tileSetSprite->Draw(surfaceLeft, static_cast<int>(tileX - offsetLeft), tileY);
		tileSetSprite->Draw(surfaceRight, static_cast<int>(tileX - offsetRight), tileY);
	}
	else
	{
		tileSetSprite->Draw(surface, static_cast<int>(tileX - offset), tileY);
	}

	// Remove collisions of destroyed breakable block
	for (int i = 0; i < wallIndex; ++i)
	{   // Check if current tilePos values are the same as destroyed breakable block
		if (tilePos[i]->pos.x == tileX &&
			tilePos[i]->pos.y == tileY)
		{
			delete tilePos[i];
			// Clear up gap in tilePos because of deleting it
			for (int j = i; j < wallIndex - 1; ++j)
			{
				tilePos[j] = tilePos[j + 1];
			}
			wallIndex--;
			break;
		}
	}
	
	int random = rand() % 15; // Makes for a 13.33% chance of getting a powerUp
	
	if (random == 1 && !doorSpawned || activeBreakableBlocks == 1)
	{
		std::cout << "\nNext Level Door has appeared!" << "\n";

		doorX = tileX;
		doorY = tileY - UI_OFFSET;

		doorSpawned = true;
	}

	if (random == 2)
	{
		std::cout << "\nEXPLOSION_RANGE Powerup spawned!" << "\n";

		powerUpX = tileX;
		powerUpY = tileY;
		
		powerUps[powerUpIndex] = new Tile(int2(powerUpX, powerUpY), true);
		powerUps[powerUpIndex]->powerUpType = PowerUpType::EXPLOSION_RANGE;
		powerUpIndex++;
	}
	if (random == 3)
	{
		std::cout << "\nEXTRA_BOMB Powerup spawned!" << "\n";

		powerUpX = tileX;
		powerUpY = tileY;

		powerUps[powerUpIndex] = new Tile(int2(powerUpX, powerUpY), true);
		powerUps[powerUpIndex]->powerUpType = PowerUpType::EXTRA_BOMB;
		powerUpIndex++;
	}
}

void Map::SpawnPowerUp()
{
	for (int i = 0; i < powerUpIndex; ++i)
	{
		switch (powerUps[i]->powerUpType)
		{
		case PowerUpType::EXPLOSION_RANGE:
			tileSetSprite->SetFrame(11);
			break;
		case PowerUpType::EXTRA_BOMB:
			tileSetSprite->SetFrame(10);
			break;
		}

		if (!m_game->singlePlayer)
		{
			tileSetSprite->Draw(surfaceLeft, static_cast<int>(powerUps[i]->pos.x - offsetLeft), powerUps[i]->pos.y);
			tileSetSprite->Draw(surfaceRight, static_cast<int>(powerUps[i]->pos.x - offsetRight), powerUps[i]->pos.y);
		}
		else
		{
			tileSetSprite->Draw(surface, static_cast<int>(powerUps[i]->pos.x - offset), powerUps[i]->pos.y);
		}
	}
}

void Map::Enemies(float deltaTime)
{
	for (int i = 0; i < enemyIndex; ++i)
	{
		Enemy* enemy = enemies[i];

		if (enemy->startDeathAnim)
		{
			if (enemy->type == EnemyType::BALLOON)
			{
				enemy->EnemyDeath(6, 10);
			}

			if (enemy->type == EnemyType::ONION)
			{
				enemy->EnemyDeath(17, 21);
			}
		}
		else
		{
			enemy->EnemyMove(deltaTime);

			if (enemy->type == EnemyType::BALLOON)
			{
				enemy->EnemyAnim(3, 0);
			}

			if (enemy->type == EnemyType::ONION)
			{
				enemy->EnemyAnim(14, 11);
			}
		}

		if (enemy->readyToDelete)
		{
			if (activeEnemies <= 0)
			{
				m_am->PlaySFX("sfx/All_Enemies_Defeated_SFX.mp3", 1.0f);
			}
			delete enemies[i];
			for (int j = i; j < enemyIndex - 1; ++j)
			{
				enemies[j] = enemies[j + 1];
			}
			enemyIndex--;
			break;
		}

		// Draw Enemies
		enemiesSprite->SetFrame(enemies[i]->frameCount);

		if (!m_game->singlePlayer)
		{
			enemiesSprite->Draw(surfaceLeft, static_cast<int>(enemies[i]->pos.x - offsetLeft), enemies[i]->pos.y);
			enemiesSprite->Draw(surfaceRight, static_cast<int>(enemies[i]->pos.x - offsetRight), enemies[i]->pos.y);
		}
		else
		{
			enemiesSprite->Draw(surface, static_cast<int>(enemies[i]->pos.x - offset), enemies[i]->pos.y);
		}

		// Debugging box for Enemies
		if (enable_debugging && !m_game->singlePlayer)
		{
			DrawDebugBox
			(
				static_cast<int>(enemies[i]->pos.x + playerSize / 2),
				static_cast<int>(enemies[i]->pos.y + playerSize / 2 - UI_OFFSET),
				static_cast<int>(playerSize),
				static_cast<int>(playerSize),
				0x00FF00
			);
		}
	}
}

bool Map::EnemyAABB(float2 playerPos, float2 playerSize, float2 enemyPos, float2 enemySize)
{
	return (playerPos.x < enemyPos.x + enemySize.x &&
			playerPos.x + playerSize.x > enemyPos.x &&
			playerPos.y < enemyPos.y + enemySize.y &&
			playerPos.y + playerSize.y > enemyPos.y);
}

void Map::GenerateEnemies()
{
	// Using the while loop with rand() is not very efficient, but I don't have the time to optimize it
	while (activeEnemies < spawn_balloon + spawn_onion)
	{
		for (int x = 0; x < 31; ++x)
		{
			for (int y = 0; y < 13; ++y)
			{
				int random = rand() % 100;

				if (mapLayout[y][x] == '-' && random == 0)
				{
					int posX = tileSize * x;
					int posY = tileSize * y + UI_OFFSET;

					EnemyType type;
					int startFrame = 0;

					if (activeEnemies < spawn_balloon)
					{
						type = EnemyType::BALLOON;
					}
					else if (activeEnemies < spawn_balloon + spawn_onion)
					{
						type = EnemyType::ONION;
						startFrame = 11;
					}

					enemies[enemyIndex] = new Enemy(m_game, this, int2(posX, posY), type, startFrame);
					enemyIndex++;
					activeEnemies++;

					if (activeEnemies >= spawn_balloon + spawn_onion)
					{
						return;
					}
				}
			}
		}
	}
}

void Map::NextLevelDoor()
{
	if (doorSpawned)
	{
		tileSetSprite->SetFrame(9);

		if (!m_game->singlePlayer)
		{
			tileSetSprite->Draw(surfaceLeft, static_cast<int>(doorX - offsetLeft), doorY + UI_OFFSET);
			tileSetSprite->Draw(surfaceRight, static_cast<int>(doorX - offsetRight), doorY + UI_OFFSET);
		}
		else
		{
			tileSetSprite->Draw(surface, static_cast<int>(doorX - offset), doorY + UI_OFFSET);
		}
	}
}

void Map::DrawDebugBox(int2 pos, int2 size, uint color)
{
	if (enable_debugging && !m_game->singlePlayer)
	{
		surfaceLeft->Box
		(
			static_cast<int>(pos.x - size.x / 2 - offsetLeft),
			pos.y - size.y / 2 + UI_OFFSET,
			static_cast<int>(pos.x + size.x / 2 - offsetLeft),
			pos.y + size.y / 2 + UI_OFFSET,
			color
		);

		surfaceRight->Box
		(
			static_cast<int>(pos.x - size.x / 2 - offsetRight),
			pos.y - size.y / 2 + UI_OFFSET,
			static_cast<int>(pos.x + size.x / 2 - offsetRight),
			pos.y + size.y / 2 + UI_OFFSET,
			color
		);
	}
}

void Map::DrawDebugBox(int posX, int posY, int sizeX, int sizeY, uint color)
{
	DrawDebugBox(int2(posX, posY), int2(sizeX, sizeY), color);
}