#ifndef L_SPRITE
#define L_SPRITE

#include "../GenericContainer.h"
#include "RSC_Texture.h"

#include <rapidxml.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

// SpriteContainer owns sprites
// sprites own animations
// animations own images

enum AnimationLoadTag {
  LOAD_TAG_UNK = 0,
  LOAD_TAG_ANIMATION = 1,
  LOAD_TAG_ANIMATION_SEQUENCE = 2
};

class RSC_Sprite;
struct Image {
  Image(const Rect &r, float len);
  Rect rect;
  float frameLength;
};

class LAnimation {
  typedef std::vector<Image> imageVec;
  friend RSC_Sprite;

 public:
  LAnimation(const double &, AnimationLoadTag t);

  ~LAnimation();
  void Clear();

  void AppendImage(const Rect &rect, float frameLength);
  float GetMaxTime() const;
  ///Returns how much time must elapse to reach the specified index
  float GetTime(int index) const;
  int GetFrameFromTimeElapsed(float time) const;

  imageVec::iterator ItBegin() { return images.begin(); }
  imageVec::iterator ItEnd() { return images.end(); }

  const Rect &GetRectAtIndex(int index) const;

  int GetWidth(int index) const;
  int GetHeight(int index) const;
  double GetSpeed() const { return defaultSpeed; }

  float GetUVTop(int index) const;
  float GetUVLeft(int index) const;
  float GetUVBottom(int index) const;
  float GetUVRight(int index) const;
  /// Gets random UVCoords bounded by a frame index coords
  std::pair<Coord2df, Coord2df> GetUVRandom(int index) const;

  int NumberOfImages() const { return images.size(); }

  void SetColorKey(int image, unsigned int r, unsigned int g, unsigned int b);

  const AnimationLoadTag loadTag;

 protected:
  /// Called after all frames have been added; Calculates UV coords for each
  /// frame
  void CalculateUV(int textureWidth, int textureHeight);

  void DeleteImages();

  /// Returns true if index is actually a valid frame index
  bool ValidateIndex(int index) const;
  imageVec images;
  int currentImage;

  double defaultSpeed;
  bool isUVCalculated;

  /// stores the uv coordinates for each frame
  /// Pair is stored as <(Left, Top), (Right, Bottom)>
  std::vector<std::pair<Coord2df, Coord2df>> UVCoords;
};

class RSC_Sprite {
  typedef std::map<std::string, LAnimation> aniMap;
  typedef aniMap::iterator aniMapIt;

 public:
  RSC_Sprite(const std::string &sname);
  ~RSC_Sprite();

  std::string GetName() { return spriteName; }

  const LAnimation *GetAnimation(const std::string &aniName) const;

  aniMapIt ItBegin() { return animations.begin(); }
  aniMapIt ItEnd() { return animations.end(); }

  LOrigin GetOrigin() const { return origin; }
  void SetOrigin(LOrigin o);

  std::string GetTextureName() const { return mTextureName; }
  std::string GetTextureNormalName() const { return mTextureNormalName; }

  void SetColorKey(const std::string &aniName, unsigned int image,
                   unsigned int r, unsigned int g, unsigned int b);

  const aniMap *GetAnimationMapPointer() const { return &animations; }

  bool LoadFromXML(const char *dat, unsigned int fsize);

  int GetWidth() const { return width; }
  int GetHeight() const { return height; }

  int GetTransparentColorRed() const { return transparentColorRed; }
  int GetTransparentColorGreen() const { return transparentColorGreen; }
  int GetTransparentColorBlue() const { return transparentColorBlue; }

  static std::unique_ptr<RSC_Sprite> LoadResource(const std::string &fname);

 protected:
  void DeleteAnimations();

  LOrigin origin;

  aniMap animations;
  std::string spriteName;
  std::string mTextureName;
  std::string mTextureNormalName;

  int width, height;  // for calculating center of image
  int transparentColorRed;
  int transparentColorGreen;
  int transparentColorBlue;

 private:
  LAnimation *LoadAnimation(rapidxml::xml_node<> *animationNode);
  LAnimation *LoadAnimationSequence(rapidxml::xml_node<> *animationNode);
};

#endif
