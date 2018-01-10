#ifndef L_SPRITECOMP
#define L_SPRITECOMP

#include <memory>
#include <functional>

#include "../BaseComponentManager.h"
#include "../Event.h"
#include "../RenderManager.h"
#include "../Resources/RSC_Sprite.h"
#include "CompPosition.h"

#include "../LuaInclude.h"

class ComponentSpriteManager;

struct AnimationData {
  typedef std::function<void()> Callback;
  typedef const std::map<std::string, LAnimation> TAnimationMap;
  AnimationData(const RSC_Sprite *sprite);

  void Update();
  bool SetAnimation(const std::string &aniName);
  bool SetAnimationPlayOnce(const std::string &aniName, Callback callback);
  bool SetImageIndex(const int &imageIndex);
  const std::string &GetAnimation() { return currentAnimationName; }
  int GetImageIndex() { return currentImageIndex; }

  float animationSpeed;
  float defaultAnimationSpeed;

  const TAnimationMap *animations;

 private:
  bool animate;
  bool playOnce;
  Callback playOnceCallback;
  int currentImageIndex;
  float currentTime;
  float maxTime;
  int maxFrames;
  std::string currentAnimationName;
  const LAnimation *currentAnimation;
};

class ComponentSprite;

class Sprite {
  friend ComponentSprite;

 public:
  Sprite(RenderManager* rm, const RSC_Sprite *sprite, MAP_DEPTH depth);

  void SetAnimation(const std::string &animationName);
  void AnimationPlayOnce(const std::string &animationName,
                         luabridge::LuaRef callback);
  void SetAnimationSpeed(float speed);
  float GetAnimationSpeed();
  void SetImageIndex(int imageIndex);
  int GetImageIndex();

  /// Whether this sprite is rendered
  void Render(bool render);

  /// Set rotation of sprite
  void SetRotation(float rotation);
  /// Set Scaling of Sprite
  void SetScaling(float scalingX, float scalingY);
  void SetScalingX(float scalingX);
  void SetScalingY(float scalingY);

  void SetOffset(float x, float y);

  void CalculateVerticies();

  float DefaultAnimationSpeed();

 protected:
  AnimationData mAnimation;
  /// Resource this Sprite is based off of
  const RSC_Sprite *mSpriteRSC;
  /** Will Add / Remove itself from the appropriate sprite batch at
   *  instantiation and destruction
   */
  RenderableSpriteBatch::Sprite mRenderableSprite;
};

class ComponentSprite : public BaseComponent {
  friend class ComponentSpriteManager;

 public:
  ComponentSprite(EID id, ComponentPosition *pos, RenderManager *r,
                  ComponentSpriteManager *manager);
  ~ComponentSprite();

  void Update();
  void HandleEvent(const Event *event);

  /// returns sprite index
  Sprite *AddSprite(const RSC_Sprite *sprite, MAP_DEPTH depth);

  /**
   * Max size needs to be set to reserve memory in the sprite vector
   * If the vector resizes and needs to reallocate memory, all references
   * to its contents become invalid.
   * Therefore, if you need more than MAX_SPRITES_DEFAULT, call this function
   * with the desired number of sprites BEFORE creating any sprites
   */
  void SetMaxSprites(unsigned int spriteCount);

 protected:
  ComponentPosition *myPos;
  RenderManager *rm;

 private:
  /// Maximum number of sprites allowed for this entity
  unsigned int mMaxSprites;
  std::vector<Sprite> mSprites;

  static unsigned int MAX_SPRITES_DEFAULT;
};

class ComponentSpriteManager
    : public BaseComponentManager_Impl<ComponentSprite> {
 public:
  ComponentSpriteManager(EventDispatcher *e);
  void SetDependencies(ComponentPositionManager *pos, RenderManager *rm);

  ComponentPositionManager *dependencyPosition;
  RenderManager *dependencyRenderManager;
  ;

  std::unique_ptr<ComponentSprite> ConstructComponent(EID id,
                                                      ComponentSprite *parent);
};

/*
=========================================================
===Quick Note about LuaBridge and Multiple Inheritence===
=========================================================
After trying to expose ComponentSprite to luabridge by declaring it as a derived
class of
RenderableObject, I would encounter an inexplicable segfault everytime I tried
to run certain
RenderableObject functions. The functions that didn't crash behaved very
strangely

As it turns out I had declared ComponentSprite's inheritence as;
class ComponentSprite : public BaseComponent, public RenderableObject

Because I declared RenderableObject SECOND, LuaBridge couldn't make sense of it
Changing the order of inheritence fixed the problem completely

*/

#endif
