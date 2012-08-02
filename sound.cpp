#include "main.hpp"
#include "sound.hpp"
#include <SDL_mixer.h>
#include <cassert>

static void cleanupSound();

static Mix_Music *music = NULL;

/* Audio settings */
static int audio_rate = 22050;
static Uint16 audio_format = AUDIO_S16; /* 16-bit stereo */
static int audio_channels = 2;
static int audio_buffers = 4096;

std::map<std::string, Mix_Chunk*> soundPool;

Sound::Sound()
{
	chunk = NULL;
}

Sound::Sound(const char *filename) 
{
	std::string filenameStr(filename);
	
	if(soundPool.find(filenameStr) != soundPool.end())
		chunk = soundPool[filenameStr];
	else
		chunk = soundPool[filenameStr] = Mix_LoadWAV(filename);
}

Sound::~Sound() 
{
}

/// 0 repetitions to play once, -1 for infinite
/// v is a volume between  and ,  is max volume
void Sound::play(int repetitions, int volume)
{
#ifdef USE_SOUND
	if(!chunk) return;
	int channel = Mix_PlayChannel(-1, chunk, repetitions);
	Mix_Volume(channel, volume);
#endif
}


float Sound::getLength()
{
	return (chunk->alen / audio_rate) / 2.0;
}


void initSound()
{
	// Initialize SDL_Mixer
	if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
		fprintf(stderr, "Failed to initialize SDL_Mixer: %s\n", Mix_GetError());
		exit(1);
	}
	atexit(cleanupSound);
	Mix_QuerySpec(&audio_rate, &audio_format, &audio_buffers);
}


void playSoundtrack(const char *filename)
{
#ifdef USE_SOUND
	if(music == NULL) {
		music = Mix_LoadMUS(filename);   // Load the music
		Mix_PlayMusic(music, -1);        // Loop infinitely
		Mix_VolumeMusic(10);             // Set volume
    } else {
		//TODO: add a transition between soundtracks
		stopSoundtrack();
		playSoundtrack(filename);
	}
#endif
}

void playSound(int x, int y, Sound *s)
{
#ifdef USE_SOUND
	if(engine && engine->view)
		engine->view->playSoundAt(x, y, s);
#endif
}

void stopSoundtrack()
{
	if(music != NULL) {
		// Stop the music from playing
		Mix_HaltMusic();

		// Unload it from memory, since we don't need it anymore
		Mix_FreeMusic(music);
		music = NULL;
	}
}

static void cleanupSound()
{
	stopSoundtrack();
	Mix_CloseAudio();
}
