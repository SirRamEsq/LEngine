#ifndef LENGINE_RENDERABLE_OBJECT_SPRITE
#define LENGINE_RENDERABLE_OBJECT_SPRITE

#include "../glslHelper.h"
#include "RenderableObject.h"
#include "VAOWrapper.h"

#include <set>

class RSC_Texture;

class RenderSpriteBatch : public RenderableObjectWorld {
  // Class constructor is protected, only meant to be initialized by
  // RenderManager
  // Class does NOT own RenderableSprites;
  friend class RenderManager;

 public:
  struct Sprite {
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

    Sprite(RenderManager *rm, const RSC_Texture *tex,
           const RSC_Texture *texNormal, const unsigned int &w,
           const unsigned int &h, const MAP_DEPTH &d,
           const Vec2 &off = Vec2(
               0.0f, 0.0f));  // Have class auto register with a sprite batch;
    ~Sprite();

    Data data;
    bool isActive;
    const RSC_Texture *texture;
    const RSC_Texture *textureNormal;
    MAP_DEPTH depth;

    unsigned int textureWidth;
    unsigned int textureHeight;
    float scaleX;
    float scaleY;
    float rotation;
    Vec2 offset;

    RenderSpriteBatch *spriteBatch;
  };

  bool isTransparent();
  void Render(const RenderCamera *camera, const RSC_GLProgram *program);

  void AddSprite(Sprite *sprite);
  void DeleteSprite(Sprite *sprite);
  bool CanAddSprites(const int &numSprites);

 protected:
  RenderSpriteBatch(RenderManager *rm, const RSC_Texture *tex,
                    const RSC_Texture *texNormal, const unsigned int &maxSize);

 private:
  VAOWrapper2D vao;
  const unsigned int maxSprites;
  unsigned int currentSize;

  const RSC_Texture *texture;
  const RSC_Texture *textureNormal;

  std::set<Sprite *> sprites;
};

#endif  // LENGINE_RENDERABLE_OBJECT_SPRITE
