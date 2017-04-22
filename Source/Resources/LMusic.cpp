#include "LMusic.h"

LMusic::LMusic(std::string name, char* data, unsigned int dataSize) : musicName(name){
    SDL_RWops *rw = SDL_RWFromMem(data, dataSize);
	Mix_Chunk *c = Mix_LoadWAV_RW(rw, 1); //May only load WAV

	chunk=c;
	SDL_FreeRW(rw);
}

LMusic::~LMusic(){
    Mix_FreeChunk(chunk);
}
