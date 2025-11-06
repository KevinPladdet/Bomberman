#include "precomp.h"
#include "bomb.h"
#include "game.h"
#include "map.h"
#include "audiomanager.h"
#include "player.h"
#include "enemy.h"
#include "iostream"

Bomb::Bomb(Game* m_game, Map* m_map, AudioManager* m_am, Player* m_player, Player* m_playerTwo, float x, float y)
	:
	m_game(m_game),
	m_map(m_map),
	m_am(m_am),
	m_player(m_player),
	m_playerTwo(m_playerTwo),
	bombPos(x, y)
{
	bombSprite = new Sprite(new Surface("assets/Bomb_48.png"), 3);
	bombBlastSprite = new Sprite(new Surface("assets/Explosion_Spritesheet_48.png"), 36);
	breakableBlockSprite = new Sprite(new Surface("assets/Tileset_Spritesheet_48.png"), 24);
	bombSprite->SetFrame(0);
	animTimer.reset();

	// Save center of bomb to bombPositions array
	int bombTileX = static_cast<int>((bombPos.x + playerSizeOffset) / m_map->tileSize);
	int bombTileY = static_cast<int>((bombPos.y + playerSizeOffset - UI_OFFSET) / m_map->tileSize);
	
	for (int i = 0; i < m_map->BOMB_AMOUNT; ++i)
	{
		if (m_map->bombPositions[i].x == -1 && m_map->bombPositions[i].y == -1)
		{
			m_map->bombPositions[i] = { bombTileX, bombTileY };
			break;
		}
	}
}

Bomb::~Bomb()
{
	delete bombSprite;
	delete bombBlastSprite;
	delete breakableBlockSprite;

	bombSprite = nullptr;
	bombBlastSprite = nullptr;
	breakableBlockSprite = nullptr;
}

void Bomb::PlaceBomb(float playerX, float playerY)
{
	m_am->PlaySFX("sfx/Place_Bomb_SFX.mp3", 1.0f);

	int tileSize = 48; // Px size of Tileset_Spritesheet_48.png

	// Gets the top left of the current tile
	int tileX = static_cast<int>((playerX + playerSizeOffset) / tileSize);
	int tileY = static_cast<int>((playerY + playerSizeOffset) / tileSize);
	// Puts the bomb at the middle of the tileX and tileY
	float bombX = static_cast<float>(tileX * tileSize + tileSize / 2 - playerSizeOffset);
	float bombY = static_cast<float>(tileY * tileSize + tileSize / 2 - playerSizeOffset);

	if (!bombPlaced)
	{
		bombPlaced = true;
		bombPos = { bombX, bombY };
		animIndex = 0;
        animTimer.reset();

		exitedByPlayer[0] = false;
		exitedByPlayer[1] = false;
	}
}

void Bomb::Update()
{
	BombAnim();

	if (!blastCompleted)
	{
		ExplodeInUpdate(bombPos);
		BombBlastAnim();
	}

	if (bombPlaced)
	{	
		if (!m_game->singlePlayer)
		{
			bombSprite->Draw(m_map->surfaceLeft, static_cast<int>(bombPos.x - m_map->offsetLeft), static_cast<int>(bombPos.y));
			bombSprite->Draw(m_map->surfaceRight, static_cast<int>(bombPos.x - m_map->offsetRight), static_cast<int>(bombPos.y));
		}
		else
		{
			bombSprite->Draw(m_map->surface, static_cast<int>(bombPos.x - m_map->offset), static_cast<int>(bombPos.y));
		}

		// Debugging hitboxes
		if (m_map->enable_debugging && !m_game->singlePlayer)
		{
			m_map->surfaceLeft->Box(static_cast<int>(bombPos.x - m_map->offsetLeft), static_cast<int>(bombPos.y), static_cast<int>(bombPos.x + bombSize.x - m_map->offsetLeft), static_cast<int>(bombPos.y + bombSize.y), 0x00FF00);
			m_map->surfaceRight->Box(static_cast<int>(bombPos.x - m_map->offsetRight), static_cast<int>(bombPos.y), static_cast<int>(bombPos.x + bombSize.x - m_map->offsetRight), static_cast<int>(bombPos.y + bombSize.y), 0x00FF00);
		}
	}
}

void Bomb::BombAnim()
{
	if (bombPlaced && animTimer.elapsed() > timePerFrame)
	{
		animIndex++;
		animTimer.reset();

		if (animIndex >= frameSequence.size())
		{
			bombPlaced = false;
			blastAnimTimer.reset();

			bbAnimIndex = 0;
			bbAnimTimer.reset();

			m_am->PlaySFX("sfx/Explode_Bomb_SFX.mp3", 1.0f);

			// Because of this BombBlastAnim() is now active in Update()
			blastCompleted = false;
		}
	}
	if (bombPlaced)
	{
		bombSprite->SetFrame(frameSequence[animIndex]);
	}
}

bool Bomb::AABB(float2 playerPos, float2 playerSize, float2 bombPos, float2 bombSize)
{
	return (playerPos.x < bombPos.x + bombSize.x &&
			playerPos.x + playerSize.x > bombPos.x &&
			playerPos.y < bombPos.y + bombSize.y &&
			playerPos.y + playerSize.y > bombPos.y);
}

bool Bomb::CheckBombCollision(Player* m_collidingPlayer, bool isPlayerTwo)
{
	float2 playerPos = { m_collidingPlayer->playerX, m_collidingPlayer->playerY };
	float2 playerSize = { m_collidingPlayer->playerSize };
	
	if (bombPlaced)
	{
		bool inside = AABB(playerPos, playerSize, bombPos, bombSize);
		
		int index = isPlayerTwo ? 1 : 0;

		if (!inside)
		{
			exitedByPlayer[index] = true; // Turns true when player goes outside of bomb collisions
		}
		
		if (exitedByPlayer[index] && inside)
		{
			return true; // If player tries to walk back inside: return true
		}
	}

	return false;
}

void Bomb::Explode(float2 bombPos)
{
	int tileX = static_cast<int>((bombPos.x + playerSizeOffset) / m_map->tileSize);
	int tileY = static_cast<int>((bombPos.y + playerSizeOffset - UI_OFFSET) / m_map->tileSize);

	// All 4 directions for the bomb to check tiles for any breakable blocks
	int directions[4][2] = 
	{
		{ -1, 0 }, // Left
		{ 1, 0 },  // Right
		{ 0, -1 }, // Up
		{ 0, 1 }   // Down
	};
	
	for (int i = 0; i < 4; i++)
	{
		int dirX = directions[i][0];
		int dirY = directions[i][1];

		for (int r = 1; r <= m_map->explosionRange; r++)
		{
			int checkX = tileX + dirX * r;
			int checkY = tileY + dirY * r;

			// Wall collision check
			if (m_map->mapLayout[checkY][checkX] == '#')
			{
				break; // Stop checking further because there is a wall
			}
			
			// Breakable Block collision check
			if (m_map->mapLayout[checkY][checkX] == 'B')
			{
				m_map->DestroyBreakableBlock(checkX, checkY);
				break;
			}
		}
	}
}

void Bomb::ExplodeInUpdate(float2 bombPos)
{
	int tileX = static_cast<int>((bombPos.x + playerSizeOffset) / m_map->tileSize);
	int tileY = static_cast<int>((bombPos.y + playerSizeOffset - UI_OFFSET) / m_map->tileSize);

	// All 4 directions for the bomb to check tiles for any breakable blocks
	int directions[5][2] =
	{
		{ 0, 0 },  // Center
		{ -1, 0 }, // Left
		{ 1, 0 },  // Right
		{ 0, -1 }, // Up
		{ 0, 1 }   // Down
	};

	for (int i = 0; i < 5; i++)
	{
		int dirX = directions[i][0];
		int dirY = directions[i][1];

		for (int r = 1; r <= m_map->explosionRange; r++)
		{
			int checkX = (tileX + dirX * r);
			int checkY = (tileY + dirY * r);

			// Wall collision check
			if (m_map->mapLayout[checkY][checkX] == '#' || m_map->mapLayout[checkY][checkX] == 'B')
			{
				break; // Stop checking further because there is a wall or block
			}

			#pragma region P1 Collision Check
			// Player1 collision check
			int p1TileX = static_cast<int>((m_player->playerX + playerSizeOffset) / m_map->tileSize);
			int p1TileY = static_cast<int>((m_player->playerY + playerSizeOffset - UI_OFFSET) / m_map->tileSize);

			m_map->DrawDebugBox(p1TileX * m_map->tileSize + playerSizeOffset, p1TileY * m_map->tileSize + playerSizeOffset, playerSizeOffset * 2, playerSizeOffset * 2, 0xFF0000);
			m_map->DrawDebugBox(checkX * m_map->tileSize + playerSizeOffset, checkY * m_map->tileSize + playerSizeOffset, playerSizeOffset * 2, playerSizeOffset * 2, 0xFFFFFF);

			if (checkX == p1TileX && checkY == p1TileY)
			{
				m_player->CallOnceOnHit();
				break;
			}
			#pragma endregion

			#pragma region P2 Collision Check
			// Player2 collision check
			if (!m_game->singlePlayer)
			{
				int p2TileX = static_cast<int>((m_playerTwo->playerX + playerSizeOffset) / m_map->tileSize);
				int p2TileY = static_cast<int>((m_playerTwo->playerY + playerSizeOffset - UI_OFFSET) / m_map->tileSize);

				m_map->DrawDebugBox(p2TileX * m_map->tileSize + playerSizeOffset, p2TileY * m_map->tileSize + playerSizeOffset, playerSizeOffset * 2, playerSizeOffset * 2, 0xFF0000);
				m_map->DrawDebugBox(checkX * m_map->tileSize + playerSizeOffset, checkY * m_map->tileSize + playerSizeOffset, playerSizeOffset * 2, playerSizeOffset * 2, 0xFFFFFF);

				if (checkX == p2TileX && checkY == p2TileY)
				{
					m_playerTwo->CallOnceOnHit();
					break;
				}
			}
			#pragma endregion

			#pragma region Enemy Collision Check
			for (int i = 0; i < m_map->enemyIndex; ++i)
			{
				int enemyTileX = static_cast<int>((m_map->enemies[i]->pos.x + playerSizeOffset) / m_map->tileSize);
				int enemyTileY = static_cast<int>((m_map->enemies[i]->pos.y + playerSizeOffset - UI_OFFSET) / m_map->tileSize);
				
				int2 bombCenter = { checkX * m_map->tileSize, checkY * m_map->tileSize };
				int2 enemyCenter = { m_map->enemies[i]->pos.x, m_map->enemies[i]->pos.y - UI_OFFSET };
				float distance = length(enemyCenter - bombCenter);

				m_map->DrawDebugBox(enemyTileX * m_map->tileSize + playerSizeOffset, enemyTileY * m_map->tileSize + playerSizeOffset, playerSizeOffset * 2, playerSizeOffset * 2, 0xFF0000);
				
				if (checkX == enemyTileX && checkY == enemyTileY && distance <= playerSizeOffset * 2 * 0.8) // playerSizeOffset * 2 (enemy size) * 0.8 (makes it so the enemy needs to be closer to get hit)
				{
					m_map->enemies[i]->startDeathAnim = true;
					break;
				}
			}
			#pragma endregion
		}
	}
}

void Bomb::BombBlastAnim()
{
	int tileX = static_cast<int>((bombPos.x + playerSizeOffset) / m_map->tileSize);
	int tileY = static_cast<int>((bombPos.y + playerSizeOffset - UI_OFFSET) / m_map->tileSize);

	if (blastAnimTimer.elapsed() > blastDurationAnim)
	{
		Explode(bombPos);

		// Clear from bombPositions so enemy can go through it
		for (int i = 0; i < m_map->BOMB_AMOUNT; ++i)
		{
			if (m_map->bombPositions[i].x == tileX && m_map->bombPositions[i].y == tileY)
			{
				m_map->bombPositions[i] = { -1, -1 }; // Mark as empty again
				break;
			}
		}

		isDone = true;
		return;
	}

	int drawTileX = tileX * m_map->tileSize;
	int drawTileY = (tileY * m_map->tileSize) + m_map->tileSize * 2;

	bool endRange = false;

	BombBlastDirection('M', endRange);
	if (!m_game->singlePlayer)
	{
		bombBlastSprite->Draw(m_map->surfaceLeft, static_cast<int>(drawTileX - m_map->offsetLeft), drawTileY);
		bombBlastSprite->Draw(m_map->surfaceRight, static_cast<int>(drawTileX - m_map->offsetRight), drawTileY);
	}
	else
	{
		bombBlastSprite->Draw(m_map->surface, static_cast<int>(drawTileX - m_map->offset), drawTileY);
	}

	// All 4 directions for the bomb to check tiles for any breakable blocks
	int directions[4][2] = {
		{ -1, 0 }, // Left
		{ 1, 0 },  // Right
		{ 0, -1 }, // Up
		{ 0, 1 }   // Down
	};

	for (int i = 0; i < 4; i++)
	{
		int dirX = directions[i][0];
		int dirY = directions[i][1];

		// Keep checking until explosionRange (for when explosionRange is more than 1)
		for (int r = 1; r <= m_map->explosionRange; r++)
		{
			int checkX = tileX + dirX * r;
			int checkY = tileY + dirY * r;

			if (r == m_map->explosionRange)
			{
				endRange = true;
			}
			else 
			{ 
				endRange = false; 
			}

			if (m_map->mapLayout[checkY][checkX] == '-' || m_map->mapLayout[checkY][checkX] == 'S')
			{
				if (dirX == -1)
				{
					BombBlastDirection('L', endRange);
				}
				if (dirX == 1)
				{
					BombBlastDirection('R', endRange);
				}
				if (dirY == -1)
				{
					BombBlastDirection('U', endRange);
				}
				if (dirY == 1)
				{
					BombBlastDirection('D', endRange);
				}
					
				int finalDrawY = checkY * m_map->tileSize + m_map->tileSize * 2;
				
				if (!m_game->singlePlayer)
				{
					bombBlastSprite->Draw(m_map->surfaceLeft, static_cast<int>(checkX * m_map->tileSize - m_map->offsetLeft), finalDrawY);
					bombBlastSprite->Draw(m_map->surfaceRight, static_cast<int>(checkX * m_map->tileSize - m_map->offsetRight), finalDrawY);
				}
				else
				{
					bombBlastSprite->Draw(m_map->surface, static_cast<int>(checkX * m_map->tileSize - m_map->offset), finalDrawY);
				}
			}

			if (m_map->mapLayout[checkY][checkX] == '#')
			{
				break; // Stop checking because there is a wall in the way
			}

			if (m_map->mapLayout[checkY][checkX] == 'B')
			{
				BreakableBlockAnim(checkX * m_map->tileSize, (checkY * m_map->tileSize) + m_map->tileSize * 2);
				break; // Stop checking because there is a breakable block in the way
			}
		}
	}
}

void Bomb::BreakableBlockAnim(int tileX, int tileY)
{
	if (bbAnimIndex >= 6)
	{
		return;
	}

	breakableBlockSprite->SetFrame(bbFrameCount + bbAnimIndex);
	if (!m_game->singlePlayer)
	{
		breakableBlockSprite->Draw(m_map->surfaceLeft, static_cast<int>(tileX - m_map->offsetLeft), tileY);
		breakableBlockSprite->Draw(m_map->surfaceRight, static_cast<int>(tileX - m_map->offsetRight), tileY);
	}
	else
	{
		breakableBlockSprite->Draw(m_map->surface, static_cast<int>(tileX - m_map->offset), tileY);
	}

	if (bbAnimTimer.elapsed() > bbTimePerFrame)
	{
		bbAnimIndex++;
		bbAnimTimer.reset();
	}
}

void Bomb::BombBlastDirection(char direction, bool endRange)
{
	float quarter = blastDurationAnim / 4.f; // there are 4 different bomb blast sizes in the spritesheet

	switch (direction)
	{
	case 'M': // Middle
		if (blastAnimTimer.elapsed() < quarter)
		{
			bombBlastSprite->SetFrame(27);
		}
		else if (blastAnimTimer.elapsed() < quarter * 2)
		{
			bombBlastSprite->SetFrame(18);
		}
		else if (blastAnimTimer.elapsed() < quarter * 3)
		{
			bombBlastSprite->SetFrame(9);
		}
		else if (blastAnimTimer.elapsed() < quarter * 4)
		{
			bombBlastSprite->SetFrame(0);
		}
		break;
	case 'L': // Left
		if (blastAnimTimer.elapsed() < quarter)
		{
			bombBlastSprite->SetFrame(28);
			if (endRange) { bombBlastSprite->SetFrame(33); }
		}
		else if (blastAnimTimer.elapsed() < quarter * 2)
		{
			bombBlastSprite->SetFrame(19);
			if (endRange) { bombBlastSprite->SetFrame(24); }
		}
		else if (blastAnimTimer.elapsed() < quarter * 3)
		{
			bombBlastSprite->SetFrame(10);
			if (endRange) { bombBlastSprite->SetFrame(15); }
		}
		else if (blastAnimTimer.elapsed() < quarter * 4)
		{
			bombBlastSprite->SetFrame(1);
			if (endRange) { bombBlastSprite->SetFrame(6); }
		}
		break;
	case 'R': // Right
		if (blastAnimTimer.elapsed() < quarter)
		{
			bombBlastSprite->SetFrame(28);
			if (endRange) { bombBlastSprite->SetFrame(29); }
		}
		else if (blastAnimTimer.elapsed() < quarter * 2)
		{
			bombBlastSprite->SetFrame(19);
			if (endRange) { bombBlastSprite->SetFrame(20); }
		}
		else if (blastAnimTimer.elapsed() < quarter * 3)
		{
			bombBlastSprite->SetFrame(10);
			if (endRange) { bombBlastSprite->SetFrame(11); }
		}
		else if (blastAnimTimer.elapsed() < quarter * 4)
		{
			bombBlastSprite->SetFrame(1);
			if (endRange) { bombBlastSprite->SetFrame(2); }
		}
		break;
	case 'U': // Up
		if (blastAnimTimer.elapsed() < quarter)
		{
			bombBlastSprite->SetFrame(30);
			if (endRange) { bombBlastSprite->SetFrame(35); }
		}
		else if (blastAnimTimer.elapsed() < quarter * 2)
		{
			bombBlastSprite->SetFrame(21);
			if (endRange) { bombBlastSprite->SetFrame(26); }
		}
		else if (blastAnimTimer.elapsed() < quarter * 3)
		{
			bombBlastSprite->SetFrame(12);
			if (endRange) { bombBlastSprite->SetFrame(17); }
		}
		else if (blastAnimTimer.elapsed() < quarter * 4)
		{
			bombBlastSprite->SetFrame(3);
			if (endRange) { bombBlastSprite->SetFrame(8); }
		}
		break;
	case 'D': // Down
		if (blastAnimTimer.elapsed() < quarter)
		{
			bombBlastSprite->SetFrame(30);
			if (endRange) { bombBlastSprite->SetFrame(31); }
		}
		else if (blastAnimTimer.elapsed() < quarter * 2)
		{
			bombBlastSprite->SetFrame(21);
			if (endRange) { bombBlastSprite->SetFrame(22); }
		}
		else if (blastAnimTimer.elapsed() < quarter * 3)
		{
			bombBlastSprite->SetFrame(12);
			if (endRange) { bombBlastSprite->SetFrame(13); }
		}
		else if (blastAnimTimer.elapsed() < quarter * 4)
		{
			bombBlastSprite->SetFrame(3);
			if (endRange) { bombBlastSprite->SetFrame(4); }
		}
		break;
	default:
		std::cout << "Something went wrong in BombBlastDirection()" << "\n";
	}
}