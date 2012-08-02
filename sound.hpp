#ifndef SOUND_HPP
#define SOUND_HPP

#include <SDL_Mixer.h>

class Sound
{
public:
	Sound();
	Sound(const char* s);
	~Sound();
	void play(int repetitions=0, int volume=100);
	float getLength();
	
protected:
	Mix_Chunk *chunk;
};

void playSoundtrack(const char *filename);
void stopSoundtrack();

void initSound();
void playSound(int x, int y, Sound *s);

#endif
