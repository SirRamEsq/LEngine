#ifndef LENGINE_RENDERABLE_OBJECT_SPRITE
#define LENGINE_RENDERABLE_OBJECT_SPRITE

#include "../glslHelper.h"
#include "RenderableObject.h"
#include "VAOWrapper.h"

#include <set>

class RSC_Texture;
class RSC_Sprite;

class RenderableSpriteBatch : public RenderableObjectWorld {
  // Class constructor is protected, only meant to be initialized by
  // RenderManager
  // Class does NOT own RenderableSprites;
  friend class RenderManager;

 public:
  /**
   * This class will auto register with a sprite batch
   */
  class Sprite {
   public:
    struct Data {
      Vec2 vertex1;
      Vec2 vertex2;
      Vec2 vertex3;
      Vec2 vertex4;

      Vec2 vertexOrigin1;
      Vec2 vertexOrigin2;
      Vec2 vertexOrigin3;
      Vec2 vertexOrigin4;

      Vec2 texture1;
      Vec2 texture2;
      Vec2 texture3;
      Vec2 texture4;

      Vec4 color;
      Vec3 scalingRotation;
      Vec2 translate;
    };

    Sprite(const Sprite &other);
    Sprite(RenderManager *rm, const RSC_Sprite *spr, MAP_DEPTH d, Vec2 off);
    ~Sprite();

    Data data;
    bool isActive;
    const RSC_Texture *texture;
    const RSC_Texture *textureNormal;
    MAP_DEPTH depth;

    float scaleX;
    float scaleY;
    float rotation;
    Vec2 offset;

    RenderableSpriteBatch *spriteBatch;
    RenderManager *manager;
  };

  bool isTransparent();
  void Render(const RenderCamera *camera, const RSC_GLProgram *program);

  void AddSprite(Sprite *sprite);
  void DeleteSprite(Sprite *sprite);
  bool CanAddSprites(const int &numSprites);

 protected:
  RenderableSpriteBatch(RenderManager *rm, const RSC_Texture *tex,
                        const RSC_Texture *texNormal,
                        const unsigned int &maxSize);

 private:
  VAOWrapper2D vao;
  const unsigned int maxSprites;
  unsigned int currentSize;

  const RSC_Texture *texture;
  const RSC_Texture *textureNormal;

  std::set<Sprite *> sprites;
};

#endif  // LENGINE_RENDERABLE_OBJECT_SPRITE
