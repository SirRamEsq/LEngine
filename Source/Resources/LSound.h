#ifndef L_ENGINE_SOUND
#define L_ENGINE_SOUND

#include "SDL2/SDL_mixer.h"
#include <memory>
#include <string>

class LSound{
    public:
        LSound(const std::string& sname, char* data, const unsigned int& dataSize);
        ~LSound();

        void PlaySound(const int& channel, const int& repeat) const ;

        const std::string soundName;

        static std::unique_ptr<LSound> LoadResource(const std::string& fname);

    private:
        Mix_Chunk* chunk;
};

class LMusic{
    public:
        LMusic(const std::string& name, char* data, unsigned int dataSize);
        ~LMusic();
        std::string musicName;

        static std::unique_ptr<LMusic> LoadResource(const std::string& fname);

    private:
        Mix_Music* music;
};

#endif
