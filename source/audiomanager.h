#pragma once
#include "miniaudio.h"

struct ma_engine;

class AudioManager
{
public:
	AudioManager();
	~AudioManager();
	void PlaySFX(char* sfxName, float volume);
	void PlayMusic(const char* musicName, float volume, bool loop = true);
	void StopMusic();

private:
	ma_engine* engine = nullptr;

	ma_sound_group* sfx_soundGroup = nullptr;
	ma_sound_group* music_soundGroup = nullptr;

	ma_sound* sfx_sound = nullptr;
	ma_sound* music_sound = nullptr;
};