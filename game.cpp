// Template, 2024 IGAD Edition
// Get the latest version from: https://github.com/jbikker/tmpl8
// IGAD/NHTV/BUAS/UU - Jacco Bikker - 2006-2024

#include "precomp.h"
#include "game.h"
#include "audiomanager.h"
#include "textmanager.h"
#include "menu.h"
#include "map.h"
#include "player.h"
#include "bomb.h"
#include <iostream>

#ifdef _DEBUG
#include "vld.h"
#endif

void Game::Init()
{
	srand((unsigned int)(time(0)));

	m_am = new AudioManager();
	
	m_am->PlayMusic("sfx/DarkSouls3_MainMenu_SFX.mp3", 1.0f, true);

	m_map = new Map(this, m_am);
	m_map->surface = screen;

	m_textmanager = new TextManager(this, m_map, m_am);
	
	m_menu = new Menu(this, m_am, m_textmanager);

	m_menu->game_state = m_menu->MAIN_MENU;

	m_player = new Player(m_map, this, m_am);
	m_playerTwo = new Player(m_map, this, m_am);
	m_playerTwo->PlayerTwo();

	screen->Clear(0xBDBEBD);
}

// TODO: 
// - Pixel Perfect
// - Fix player snapping for holding

void Game::Tick( float deltaTime )
{
	constexpr int Escape = 27;
	if (GetAsyncKeyState(Escape) & 0x01001)
	{
		if (m_menu->game_state != m_menu->MAIN_MENU)
		{
			m_am->StopMusic();
			m_am->PlayMusic("sfx/DarkSouls3_MainMenu_SFX.mp3", 1.0f, true);
			
			if (wasPlayingSingleplayer)
			{
				m_menu->selected = 1;
			}
			if (wasPlayingMultiplayer)
			{
				m_menu->selected = 2;
			}
			
			m_menu->game_state = m_menu->MAIN_MENU;
		}
	}

	switch (m_menu->game_state)
	{
	case m_menu->MAIN_MENU:
		m_menu->InMainMenu(screen);
		break;

	case m_menu->SINGLEPLAYER:
		if (!m_map->gameOverActivated && !m_map->stageTransitionActivated)
		{
			Singleplayer(deltaTime);
			m_textmanager->DrawUI(screen);
		}
		else if (m_map->stageTransitionActivated)
		{
			screen->Clear(0x000000);
			m_am->StopMusic();
			m_am->PlayMusic("sfx/Stage_Transition_Music.mp3", 1.0f, false);
			m_menu->endOfMusicTimer.reset();
			m_menu->game_state = m_menu->STAGE_TRANSITION;
		}
		else if (m_map->gameOverActivated)
		{
			screen->Clear(0x000000);
			if (score > topScore)
			{
				topScore = score; // Set topScore when game ends
			}
			printf("- - - - - GAME OVER - - - - -\n");
			printf("You ran out of lives and died!\n");
			printf("Total Score: %d\n\n", score);
			m_am->StopMusic();
			m_am->PlayMusic("sfx/Game_Over_Music.mp3", 1.0f, false);
			m_menu->endOfMusicTimer.reset();
			m_menu->game_state = m_menu->GAME_OVER;
		}
		break;

	case m_menu->MULTIPLAYER:
		if (!m_map->gameOverActivated && !m_map->stageTransitionActivated)
		{
			Multiplayer(deltaTime);
			m_textmanager->DrawUI(screen);
		}
		else if (m_map->stageTransitionActivated)
		{
			screen->Clear(0x000000);
			m_am->StopMusic();
			m_am->PlayMusic("sfx/Stage_Transition_Music.mp3", 1.0f, false);
			m_menu->endOfMusicTimer.reset();
			m_menu->game_state = m_menu->STAGE_TRANSITION;
		}
		else if (m_map->gameOverActivated)
		{
			screen->Clear(0x000000);
			if (score > topScore)
			{
				topScore = score; // Set topScore when game ends
			}
			printf("- - - - - GAME OVER - - - - -\n");
			printf("You ran out of lives and died!\n");
			printf("Total Score: %d\n\n", score);
			m_am->StopMusic();
			m_am->PlayMusic("sfx/Game_Over_Music.mp3", 1.0f, false);
			m_menu->endOfMusicTimer.reset();
			m_menu->game_state = m_menu->GAME_OVER;
		}
		break;

	case m_menu->GAME_OVER:
		m_menu->DrawGameOverMenu(screen);
		break;

	case m_menu->STAGE_TRANSITION:
		m_menu->DrawStageTransitionMenu(screen);
		break;

	case m_menu->EXIT:
		PostQuitMessage(0);
		break;
	}
}

void Game::SpawnBomb(float x, float y, bool isPlayerTwo)
{
	int& bombCount = isPlayerTwo ? p2BombCount : p1BombCount;

	if (bombCount >= m_map->bombAmount)
	{
		return;
	}

	for (int i = 0; i < MAX_BOMBS; i++)
	{
		if (m_bombs[i] == nullptr)
		{
			m_bombs[i] = new Bomb(this, m_map, m_am, m_player, m_playerTwo, x, y);
			m_bombs[i]->ownerIsPlayerTwo = isPlayerTwo;
			m_bombs[i]->PlaceBomb(x, y);
			bombCount++;
			break;
		}
	}
}

bool Game::CheckPlayerBombCollision(Player* player, bool isPlayerTwo)
{
	for (int i = 0; i < MAX_BOMBS; i++)
	{
		if (m_bombs[i] && m_bombs[i]->CheckBombCollision(player, isPlayerTwo))
		{
			return true;
		}
	}
	return false;
}

void Game::Singleplayer(float deltaTime)
{
	m_player->Update(deltaTime);
	m_map->Splitscreen(); // Creates tilemap for player
	m_map->NextLevelDoor();
	m_map->Enemies(deltaTime); // Enemy drawing and movement
	m_map->DrawPlayers(m_player, m_playerTwo); // Draws player on surface

	// Update bombs and delete when bomb blast animation is done
	for (int i = 0; i < MAX_BOMBS; i++)
	{
		if (m_bombs[i])
		{
			m_bombs[i]->Update();
			if (m_bombs[i]->IsDone())
			{
				if (m_bombs[i]->ownerIsPlayerTwo)
				{
					p2BombCount--;
				}
				else
				{
					p1BombCount--;
				}
				delete m_bombs[i];
				m_bombs[i] = nullptr;
			}
		}
	}
}

void Game::Multiplayer(float deltaTime)
{
	m_player->Update(deltaTime);
	m_playerTwo->Update(deltaTime);

	m_map->Splitscreen(); // Creates tilemap for player and playerTwo
	m_map->NextLevelDoor();
	m_map->Enemies(deltaTime); // Enemy drawing and movement
	m_map->DrawPlayers(m_player, m_playerTwo); // Draws player and playerTwo on both surfaceLeft and surfaceRight

	// Update bombs and delete when bomb blast animation is done
	for (int i = 0; i < MAX_BOMBS; i++)
	{
		if (m_bombs[i])
		{
			m_bombs[i]->Update();
			if (m_bombs[i]->IsDone())
			{
				if (m_bombs[i]->ownerIsPlayerTwo)
				{
					p2BombCount--;
				}
				else
				{
					p1BombCount--;
				}
				delete m_bombs[i];
				m_bombs[i] = nullptr;
			}
		}
	}

	screen->Line(SCRWIDTH / 2, UI_OFFSET, SCRWIDTH / 2, SCRHEIGHT, 0x000000);
}

void Game::NewGame()
{
	if (wasPlayingSingleplayer)
	{
		m_menu->selected = 1;
	}
	if (wasPlayingMultiplayer)
	{
		m_menu->selected = 2;
	}

	score = 0;
	lives = 3;
	stage = 1;

	NewLife();
}

void Game::NewLife()
{
	timeLeft = 200;
	deathSoundPlaying = false;
	p1BombCount = 0;
	p2BombCount = 0;

	delete m_map;
	delete m_player;
	delete m_playerTwo;
	m_map = nullptr;
	m_player = nullptr;
	m_playerTwo = nullptr;

	// Delete all bombs
	for (int i = 0; i < MAX_BOMBS; i++)
	{
		delete m_bombs[i];
		m_bombs[i] = nullptr;
	}

	// Recreate map
	m_map = new Map(this, m_am);
	m_map->surface = screen;

	// Add more enemies depending on stage
	if (stage >= 2)
	{
		int spawnAmountOnions = 2;
		int increaseForEveryStage = stage - 1;
		// Increases enemies by 1 every stage
		m_map->spawn_balloon += increaseForEveryStage;
		m_map->spawn_onion += spawnAmountOnions + increaseForEveryStage;
		m_map->GenerateEnemies();
	}

	// Recreate players
	if (singlePlayer)
	{
		m_player = new Player(m_map, this, m_am);
	}
	else
	{
		m_player = new Player(m_map, this, m_am);
		m_playerTwo = new Player(m_map, this, m_am);
		m_playerTwo->PlayerTwo();
	}
	
	screen->Clear(0xBDBEBD);
}

void Game::DeleteForSingleplayer()
{
	if(m_playerTwo != nullptr)
	{
		delete m_playerTwo;
		m_playerTwo = nullptr;
	}
}

void Game::Shutdown()
{
	delete m_am;
	delete m_textmanager;
	delete m_menu;
	delete m_map;
	delete m_player;
	delete m_playerTwo;

	m_am = nullptr;
	m_textmanager = nullptr;
	m_menu = nullptr;
	m_map = nullptr;
	m_player = nullptr;
	m_playerTwo = nullptr;

	// Delete all bombs
	for (int i = 0; i < MAX_BOMBS; i++)
	{
		delete m_bombs[i];
		m_bombs[i] = nullptr;
	}
}

void Game::KeyUp(int key)
{
	m_player->ProcessInput(key, false);
}

void Game::KeyDown(int key)
{
	m_player->ProcessInput(key, true);
}