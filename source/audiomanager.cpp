#include "precomp.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "audiomanager.h"

AudioManager::AudioManager()
{
	engine = new ma_engine;
	ma_engine_init(NULL, engine);

	sfx_soundGroup = new ma_sound_group;
	music_soundGroup = new ma_sound_group;
	
	ma_sound_group_init(engine, 0, NULL, sfx_soundGroup);
	ma_sound_group_init(engine, 0, NULL, music_soundGroup);
	
	// goes from 0.0 (silent) to 1.0 (full volume)
	ma_sound_group_set_volume(sfx_soundGroup, 1.0f);
	ma_sound_group_set_volume(music_soundGroup, 1.0f);
}

AudioManager::~AudioManager()
{
	if (sfx_sound && ma_sound_is_playing(sfx_sound))
	{
		ma_sound_stop(sfx_sound);
		ma_sound_uninit(sfx_sound);
	}

	if (music_sound && ma_sound_is_playing(music_sound))
	{
		ma_sound_stop(music_sound);
		ma_sound_uninit(music_sound);
	}

	ma_sound_group_uninit(sfx_soundGroup);
	ma_sound_group_uninit(music_soundGroup);

	ma_engine_uninit(engine);

	delete sfx_sound;
	delete music_sound;
	delete sfx_soundGroup;
	delete music_soundGroup;
	delete engine;
	
	sfx_sound = nullptr;
	music_sound = nullptr;
	sfx_soundGroup = nullptr;
	music_soundGroup = nullptr;
	engine = nullptr;
}

void AudioManager::PlaySFX(char* sfxName, float volume)
{
	if (sfx_sound) 
	{
		ma_sound_uninit(sfx_sound);
		delete sfx_sound;
	}
	sfx_sound = new ma_sound();

	ma_sound_init_from_file(engine, sfxName, MA_SOUND_FLAG_DECODE, sfx_soundGroup, NULL, sfx_sound);
	ma_sound_set_volume(sfx_sound, volume);
	ma_sound_start(sfx_sound);
}

void AudioManager::PlayMusic(const char* musicName, float volume, bool loop)
{
	if (music_sound)
	{
		ma_sound_uninit(music_sound);
		delete music_sound;
	}
	music_sound = new ma_sound();

	ma_sound_init_from_file(engine, musicName, MA_SOUND_FLAG_STREAM, music_soundGroup, NULL, music_sound);
	ma_sound_set_volume(music_sound, volume);
	ma_sound_set_looping(music_sound, loop);
	ma_sound_start(music_sound);
}

void AudioManager::StopMusic()
{
	if (music_sound != nullptr)
	{
		ma_sound_stop(music_sound);
	}
}