#include "precomp.h"
#include "menu.h"
#include "game.h"
#include "audiomanager.h"
#include "textmanager.h"

Menu::Menu(Game* m_game, AudioManager* m_am, TextManager* m_textmanager)
	:
	m_game(m_game),
	m_am(m_am),
	m_textmanager(m_textmanager)
{
	mainMenu_Singleplayer_Sprite = new Sprite(new Surface("assets/MainMenu_Singleplayer.png"), 1);
	mainMenu_Multiplayer_Sprite = new Sprite(new Surface("assets/MainMenu_Multiplayer.png"), 1);
	mainMenu_Exit_Sprite = new Sprite(new Surface("assets/MainMenu_Exit.png"), 1);
}

Menu::~Menu()
{
	delete mainMenu_Singleplayer_Sprite;
	delete mainMenu_Multiplayer_Sprite;
	delete mainMenu_Exit_Sprite;

	mainMenu_Singleplayer_Sprite = nullptr;
	mainMenu_Singleplayer_Sprite = nullptr;
	mainMenu_Exit_Sprite = nullptr;
}

void Menu::InMainMenu(Surface* surface)
{
	// Set selected_state based on selected
	switch (selected)
	{
	case 1:
		selected_state = SINGLEPLAYER;
		break;

	case 2:
		selected_state = MULTIPLAYER;
		break;

	case 3:
		selected_state = EXIT;
		break;
	}

	constexpr int Enter = 13;
	if (GetAsyncKeyState(Enter) & 0x0001)
	{
		switch (selected_state)
		{
		case SINGLEPLAYER:
			m_am->StopMusic();
			m_am->PlayMusic("sfx/Background_Music2.mp3", 1.0f, true);

			m_game->singlePlayer = true;
			m_game->DeleteForSingleplayer();

			if (m_game->wasPlayingMultiplayer)
			{
				m_game->NewGame();
			}
			m_game->wasPlayingMultiplayer = false;
			m_game->wasPlayingSingleplayer = true;

			game_state = SINGLEPLAYER;
			break;

		case MULTIPLAYER:
			m_am->StopMusic();
			m_am->PlayMusic("sfx/Background_Music1.mp3", 1.0f, true);
			m_game->singlePlayer = false;

			if (m_game->wasPlayingSingleplayer)
			{
				m_game->NewGame();
			}
			m_game->wasPlayingSingleplayer = false;
			m_game->wasPlayingMultiplayer = true;

			game_state = MULTIPLAYER;
			break;

		case EXIT:
			game_state = EXIT;
			break;
		}
	}

	// Loop through selected from left
	if (GetAsyncKeyState('A') & 0x1001)
	{
		PlaySelectSFX();
		if (selected == 1)
		{
			selected = totalAmountOptions;
		}
		else
		{
			selected--;
		}
	}
	// Loop through selected from right
	if (GetAsyncKeyState('D') & 0x1001)
	{
		PlaySelectSFX();
		if (selected == totalAmountOptions)
		{
			selected = 1;
		}
		else
		{
			selected++;
		}
	}

	DrawMenu(surface);
}

void Menu::DrawMenu(Surface* surface)
{
	if (selected == 1)
	{
		mainMenu_Singleplayer_Sprite->Draw(surface, 0, 0);
	}
	else if (selected == 2)
	{
		mainMenu_Multiplayer_Sprite->Draw(surface, 0, 0);
	}
	else if (selected == 3)
	{
		mainMenu_Exit_Sprite->Draw(surface, 0, 0);
	}

	// Top Score Text
	int2 topScorePos = { 592, 504 };
	string topScoreText = to_string(m_game->topScore);
	m_textmanager->DrawText(surface, topScorePos, m_textmanager->smallTextSize, topScoreText, true);
}

void Menu::DrawGameOverMenu(Surface* surface)
{
	float endOfMusic = 6.4f;

	if (endOfMusicTimer.elapsed() > endOfMusic)
	{
		m_game->NewGame();
		m_am->StopMusic();
		m_am->PlayMusic("sfx/DarkSouls3_MainMenu_SFX.mp3", 1.0f, true);
		game_state = MAIN_MENU;
	}

	// Game Over Text
	int gameOverTextOffset = 48 * 9 / 2; // textSize * amount of characters in string / 2
	int2 gameOverPos = { SCRWIDTH / 2 - gameOverTextOffset, SCRHEIGHT / 2 - m_textmanager->bigTextSize.y };
	string gameOverText = string("game over");
	m_textmanager->DrawText(surface, gameOverPos, m_textmanager->bigTextSize, gameOverText, true);
}

void Menu::DrawStageTransitionMenu(Surface* surface)
{
	float endOfMusic = 3.3f;

	if (endOfMusicTimer.elapsed() > endOfMusic)
	{
		m_game->NewLife();
		m_am->StopMusic();
		m_am->PlayMusic("sfx/Background_Music1.mp3", 1.0f, true);
		if (m_game->wasPlayingSingleplayer)
		{
			game_state = SINGLEPLAYER;
		}
		else if (m_game->wasPlayingMultiplayer)
		{
			game_state = MULTIPLAYER;
		}
	}

	// Stage Transition Text
	int stageTextOffset = 48 * 7 / 2; // textSize * amount of characters in string / 2
	int2 stagePos = { SCRWIDTH / 2 - stageTextOffset, SCRHEIGHT / 2 - m_textmanager->bigTextSize.y };
	string stageText = string("stage ") + to_string(m_game->stage);
	m_textmanager->DrawText(surface, stagePos, m_textmanager->bigTextSize, stageText, true);
}

void Menu::PlaySelectSFX()
{
	float sfxCooldown = 0.18f;

	if (selectSfxCooldownTimer.elapsed() > sfxCooldown)
	{
		selectSfxCooldownTimer.reset();
		m_am->PlaySFX("sfx/Menu_Select_SFX.mp3", 1.0f);
	}
}