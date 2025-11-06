#pragma once

class Game;
class Map;
class AudioManager;

class TextManager
{
public:
	TextManager(Game* m_game, Map* m_map, AudioManager* m_am);
	~TextManager();
	void DrawUI(Surface* surface);
	void TimeUI(Surface* surface);
	void DrawText(Surface* surface, int2 position, int2 size, string& text, bool blackFont);

	int2 smallTextSize = { 24, 24 };
	int2 normalTextSize = { 36, 36 };
	int2 bigTextSize = { 48, 48 };

private:
	int spriteNumber = 0;

	// Time
	Timer timer;
	bool timeUp = false;

	Sprite* font_Black_Sprite = nullptr;
	Sprite* font_Grey_Sprite = nullptr;

	Game* m_game = nullptr;
	Map* m_map = nullptr;
	AudioManager* m_am = nullptr;
};