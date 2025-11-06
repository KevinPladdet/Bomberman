#include "precomp.h"
#include "textmanager.h"
#include "game.h"
#include "map.h"
#include "audiomanager.h"

TextManager::TextManager(Game* m_game, Map* m_map, AudioManager* m_am)
	:
	m_game(m_game),
	m_map(m_map),
	m_am(m_am)
{
	font_Black_Sprite = new Sprite(new Surface("assets/Font_Black.png"), 38);
	font_Grey_Sprite = new Sprite(new Surface("assets/Font_Grey.png"), 38);
}

TextManager::~TextManager()
{
	delete font_Black_Sprite;
	delete font_Grey_Sprite;
	
	font_Black_Sprite = nullptr;
	font_Grey_Sprite = nullptr;
}

void TextManager::DrawUI(Surface* surface)
{
	surface->Bar(0, 0, SCRWIDTH, UI_OFFSET, 0xBDBEBD); // Clears screen where UI is (better performance than using screen->Clear)

	if (timer.elapsed() > 1 && !timeUp)
	{
		if (m_game->timeLeft == 1)
		{
			m_am->StopMusic();
			m_am->PlayMusic("sfx/AlarmHorn_SFX.mp3", 1.0f, false);
		}
		if (m_game->timeLeft == 0)
		{
			timeUp = true;

			// Enemies will spawn because time is up
			m_map->spawn_onion += 5;
			m_map->GenerateEnemies();
		}
		else
		{
			m_game->timeLeft--;
			timer.reset();
		}
	}
	
	bool musicIsResetted = false;
	if (timer.elapsed() > 2.02f) { musicIsResetted = true; }
	if (timer.elapsed() > 2.0f && !musicIsResetted)
	{
		m_am->StopMusic();
		m_am->PlayMusic("sfx/Background_Music1.mp3", 1.0f, true);
		musicIsResetted = true;
	}

	// Score Text
	int2 scorePos = { 450, 32 };
	string scoreText = string("score ") + to_string(m_game->score);
	DrawText(surface, scorePos, normalTextSize, scoreText, false);

	// Lives Text
	int2 livesPos = { 992, 32 };
	string livesText = string("lives ") + to_string(m_game->lives);
	DrawText(surface, livesPos, normalTextSize, livesText, false);

	// Timer Text
	TimeUI(surface);
}

void TextManager::TimeUI(Surface* surface)
{
	int2 timePos = { 24, 32 };
	string scoreText = string("time ") + to_string(m_game->timeLeft);
	DrawText(surface, timePos, normalTextSize, scoreText, false);
}

// I made this code with help from Koen
void TextManager::DrawText(Surface* surface, int2 position, int2 size, string& text, bool blackFont)
{
	Sprite& tempSprite = blackFont ? *font_Black_Sprite : *font_Grey_Sprite;

	for (int i = 0; i < text.size(); i++)
	{
		
		switch (text[i])
		{
		case ' ':
			spriteNumber = 37;
			break;
		case 'a':
			spriteNumber = 11;
			break;
		case 'b':
			spriteNumber = 12;
			break;
		case 'c':
			spriteNumber = 13;
			break;
		case 'd':
			spriteNumber = 14;
			break;
		case 'e':
			spriteNumber = 15;
			break;
		case 'f':
			spriteNumber = 16;
			break;
		case 'g':
			spriteNumber = 17;
			break;
		case 'h':
			spriteNumber = 18;
			break;
		case 'i':
			spriteNumber = 19;
			break;
		case 'j':
			spriteNumber = 20;
			break;
		case 'k':
			spriteNumber = 21;
			break;
		case 'l':
			spriteNumber = 22;
			break;
		case 'm':
			spriteNumber = 23;
			break;
		case 'n':
			spriteNumber = 24;
			break;
		case 'o':
			spriteNumber = 25;
			break;
		case 'p':
			spriteNumber = 26;
			break;
		case 'q':
			spriteNumber = 27;
			break;
		case 'r':
			spriteNumber = 28;
			break;
		case 's':
			spriteNumber = 29;
			break;
		case 't':
			spriteNumber = 30;
			break;
		case 'u':
			spriteNumber = 31;
			break;
		case 'v':
			spriteNumber = 32;
			break;
		case 'w':
			spriteNumber = 33;
			break;
		case 'x':
			spriteNumber = 34;
			break;
		case 'y':
			spriteNumber = 35;
			break;
		case 'z':
			spriteNumber = 36;
			break;
		case '0':
			spriteNumber = 0;
			break;
		case '1':
			spriteNumber = 1;
			break;
		case '2':
			spriteNumber = 2;
			break;
		case '3':
			spriteNumber = 3;
			break;
		case '4':
			spriteNumber = 4;
			break;
		case '5':
			spriteNumber = 5;
			break;
		case '6':
			spriteNumber = 6;
			break;
		case '7':
			spriteNumber = 7;
			break;
		case '8':
			spriteNumber = 8;
			break;
		case '9':
			spriteNumber = 9;
			break;

		default:
			spriteNumber = 37;
			break;
		}
		
		tempSprite.SetFrame(spriteNumber);
		tempSprite.DrawScaled(position.x + i * size.x, position.y, size.x, size.y, surface);
	}
}