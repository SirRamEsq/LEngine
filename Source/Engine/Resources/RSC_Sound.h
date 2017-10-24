#ifndef L_ENGINE_SOUND
#define L_ENGINE_SOUND

#include "SDL2/SDL_mixer.h"
#include <memory>
#include <string>

class RSC_Sound {
 public:
  RSC_Sound(const std::string &sname, char *data, const unsigned int &dataSize);
  ~RSC_Sound();

  void PlaySound(const int &channel, const int &repeat) const;

  const std::string soundName;

  static std::unique_ptr<RSC_Sound> LoadResource(const std::string &fname);

 private:
  Mix_Chunk *chunk;
};

class RSC_Music {
 public:
  RSC_Music(const std::string &name, char *data, unsigned int dataSize);
  ~RSC_Music();

  void FadeIn(int timeInMS, int loops = -1) const;
  void PlayMusic(int loops = -1) const;
  bool FadeOut(int timeInMS) const;

  static int SetVolume(int volume);
  static int GetVolume();

  const std::string musicName;

  static std::unique_ptr<RSC_Music> LoadResource(const std::string &fname);

 private:
  Mix_Music *music;
};

#endif
