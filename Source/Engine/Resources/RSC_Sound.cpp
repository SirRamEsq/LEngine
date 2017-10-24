#include "RSC_Sound.h"
#include "../Defines.h"
#include "../Kernel.h"
#include "ResourceLoading.h"

//////////
// LSOUND//
//////////

RSC_Sound::RSC_Sound(const std::string &sname, char *data,
                     const unsigned int &dataSize)
    : soundName(sname) {
  SDL_RWops *rw = SDL_RWFromMem(data, dataSize);
  Mix_Chunk *c = Mix_LoadWAV_RW(rw, 1);

  chunk = c;
  // Not entirely sure if I should free this data here;
  // stuff crashes when I do.
  // May want to look for mem leak
  // SDL_FreeRW(rw);
}

void RSC_Sound::PlaySound(const int &channel = -1,
                          const int &repeat = 0) const {
  Mix_PlayChannel(channel, chunk, repeat);
}

RSC_Sound::~RSC_Sound() { Mix_FreeChunk(chunk); }

std::unique_ptr<RSC_Sound> RSC_Sound::LoadResource(const std::string &fname) {
  std::unique_ptr<RSC_Sound> sound = NULL;
  try {
    std::string fullPath = "Resources/Sounds/" + fname;
    auto data = LoadGenericFile(fullPath);
    if (data.get()->GetData() == NULL) {
      return NULL;
    }
    sound = std::make_unique<RSC_Sound>(fname, data.get()->GetData(),
                                        data.get()->length);
  } catch (LEngineFileException e) {
    LOG_INFO(e.what());
  }

  return sound;
}

//////////
// LMUSIC//
//////////

RSC_Music::RSC_Music(const std::string &name, char *data, unsigned int dataSize)
    : musicName(name) {
  SDL_RWops *rw = SDL_RWFromMem(data, dataSize);
  music = Mix_LoadMUS_RW(rw, 1);

  SDL_FreeRW(rw);
}

RSC_Music::~RSC_Music() { Mix_FreeMusic(music); }

void RSC_Music::PlayMusic(int loops) const { Mix_PlayMusic(music, loops); }

int RSC_Music::SetVolume(int volume) { return Mix_VolumeMusic(volume); }

int RSC_Music::GetVolume() { return Mix_VolumeMusic(-1); }

void RSC_Music::FadeIn(int timeInMS, int loops) const {
  Mix_FadeInMusic(music, loops, timeInMS);
}

bool RSC_Music::FadeOut(int timeInMS) const {
  return (!Mix_FadeOutMusic(timeInMS) && Mix_PlayingMusic());
}

std::unique_ptr<RSC_Music> RSC_Music::LoadResource(const std::string &fname) {
  std::unique_ptr<RSC_Music> music = NULL;
  try {
    std::string fullPath = "Resources/Music/" + fname;
    auto data = LoadGenericFile(fullPath);
    if (data.get()->GetData() == NULL) {
      return NULL;
    }
    music = std::make_unique<RSC_Music>(fname, data.get()->GetData(),
                                        data.get()->length);
  } catch (LEngineFileException e) {
    LOG_INFO(e.what());
  }

  return music;
}
