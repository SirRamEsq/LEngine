#include <sstream>

#include "AudioSubsystem.h"
#include "Kernel.h"

//////////////
//SoundEvent//
//////////////
SoundEvent::SoundEvent(const RSC_Sound* snd, unsigned short int vol)
                        : sound(snd){
    SetVolume(vol);
}

SoundEvent::SoundEvent(const std::string& soundName, unsigned short int vol){
    SetVolume(vol);

    const RSC_Sound* snd = K_SoundMan.GetItem(soundName);
    if(snd==NULL){
        K_SoundMan.LoadItem(soundName, soundName);
        snd = K_SoundMan.GetItem(soundName);
        if(snd==NULL){
            std::stringstream error;
            error << "Couldn't load sound named " << soundName;
            ErrorLog::WriteToFile(error.str(), ErrorLog::GenericLogFile);
        }
    }
    sound = snd;
}

void SoundEvent::SetVolume(unsigned short int vol){
    if(vol > 100){vol=100;}
    if(vol < 0  ){vol=0;  }

    volume=vol;
}

//////////////////
//AudioSubsystem//
//////////////////
AudioSubsystem::AudioSubsystem(){

}
AudioSubsystem::~AudioSubsystem(){

}

void AudioSubsystem::PushSoundEvent(SoundEvent event){
    soundQueue.push(event);
}

void AudioSubsystem::ProcessEvents(){
    //We have an entire frame's batch of audio events here, could lead to some interesting effects?
    while(!soundQueue.empty()){
        SoundEvent& event= soundQueue.front();
        event.sound->PlaySound(-1, 0);

        soundQueue.pop();
    }
}

void AudioSubsystem::UpdateMusic(){
	if(nextPlaying != NULL){
		if(nowPlaying != NULL){
			if(nowPlaying->FadeOut(5000) != true){
				return;
			}
		}
		nowPlaying = nextPlaying;

		RSC_Music::SetVolume(nextPlayingVolume);
		nowPlaying->FadeIn(5000, nextPlayingLoops);

		nextPlaying = NULL;
		nextPlayingLoops = -1;
		nextPlayingVolume = MIX_MAX_VOLUME;
	}
}

void AudioSubsystem::PlayMusic(const RSC_Music* music, int volume, int loops){
	nextPlaying = music;
	nextPlayingVolume = volume;
	nextPlayingLoops = loops;
}
