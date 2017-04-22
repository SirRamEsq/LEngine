#ifndef L_MUSIC
#define L_MUSIC

#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"

#include <string>

class LMusic{
    public:
        LMusic(std::string name, char* data, unsigned int dataSize);
        ~LMusic();
        std::string musicName;

    private:
        Mix_Chunk* chunk;
};

#endif
