#include "LSound.h"

//////////
//LSOUND//
//////////

LSound::LSound(const std::string& sname, char* data, const unsigned int& dataSize) : soundName(sname){
    SDL_RWops *rw = SDL_RWFromMem(data, dataSize);
	Mix_Chunk *c = Mix_LoadWAV_RW(rw, 1);

	chunk=c;
	//Not entirely sure if I should free this data here;
	//stuff crashes when I do.
	//May want to look for mem leak
	//SDL_FreeRW(rw);
}

void LSound::PlaySound(const int& channel=-1, const int& repeat=0) const {
    Mix_PlayChannel(channel, chunk, repeat);
}

LSound::~LSound(){
    Mix_FreeChunk(chunk);
}

//////////
//LMUSIC//
//////////

LMusic::LMusic(const std::string& name, char* data, unsigned int dataSize) : musicName(name){
    SDL_RWops *rw = SDL_RWFromMem(data, dataSize);
	Mix_Music *m = Mix_LoadMUS_RW(rw, 1);

	music=m;
	SDL_FreeRW(rw);
}

LMusic::~LMusic(){
    Mix_FreeMusic(music);
}
