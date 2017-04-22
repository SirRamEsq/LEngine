#ifndef L_ENGINE_AUDIO_SUB
#define L_ENGINE_AUDIO_SUB

#include <queue>

#include "Resources/LSound.h"

struct SoundEvent{
    SoundEvent(const std::string& soundName, unsigned short int vol);
    SoundEvent(const LSound* snd, unsigned short int vol);

    void SetVolume(unsigned short int vol);

    const LSound* sound;
    unsigned short int volume;
};

class AudioSubsystem{
    public:
        AudioSubsystem();
        ~AudioSubsystem();

        void ProcessEvents();

        void PushSoundEvent(SoundEvent event);

    private:
        std::queue<SoundEvent> soundQueue;
};

#endif // L_ENGINE_AUDIO_SUB
