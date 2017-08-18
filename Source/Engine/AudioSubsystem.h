#ifndef L_ENGINE_AUDIO_SUB
#define L_ENGINE_AUDIO_SUB

#include <queue>

#include "Resources/RSC_Sound.h"

//SDL Audio is already multithreaded

struct SoundEvent{
    SoundEvent(const std::string& soundName, unsigned short int vol);
    SoundEvent(const RSC_Sound* snd, unsigned short int vol);

    void SetVolume(unsigned short int vol);

    const RSC_Sound* sound;
    unsigned short int volume;
};

class AudioSubsystem{
    public:
        AudioSubsystem();
        ~AudioSubsystem();

        void ProcessEvents();
		void UpdateMusic();

        void PushSoundEvent(SoundEvent event);
		void PlayMusic(const RSC_Music* music, int volume, int loops);

    private:
		const RSC_Music* nowPlaying;
		const RSC_Music* nextPlaying;
		int nextPlayingLoops;
		int nextPlayingVolume;
        std::queue<SoundEvent> soundQueue;
};

#endif // L_ENGINE_AUDIO_SUB
