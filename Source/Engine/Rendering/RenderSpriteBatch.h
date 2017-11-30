#ifndef LENGINE_RENDERABLE_OBJECT_SPRITE
#define LENGINE_RENDERABLE_OBJECT_SPRITE

#include "../glslHelper.h"
#include "RenderableObject.h"

#include <set>

class RSC_Texture;

class VAOWrapperSprite {
 public:
  // Max size is the number of objects, not the number of verticies.
  // VAOWrapper will figure out the number of vertiices from the number of
  // objects
  VAOWrapperSprite(const unsigned int &maxSize);
  ~VAOWrapperSprite();
  void UpdateGPU();

  Vec2 *GetVertexArray() { return vboVertexArray.get(); }
  Vec2 *GetTranslateArray() { return vboTranslateArray.get(); }
  Vec2 *GetTextureArray() { return vboTextureArray.get(); }
  Vec4 *GetColorArray() { return vboColorArray.get(); }
  Vec3 *GetScalingRotationArray() { return vboScalingRotationArray.get(); }

  GLuint GetVAOID() { return vao; }

  const unsigned int vboMaxSize;

 private:
  GLuint vboVertex;
  GLuint vboTranslate;
  GLuint vboTexture;
  GLuint vboColor;
  GLuint vboScalingRotation;
  GLuint vao;

  std::unique_ptr<Vec2[]> vboVertexArray;
  std::unique_ptr<Vec2[]> vboTranslateArray;
  std::unique_ptr<Vec2[]> vboTextureArray;
  std::unique_ptr<Vec4[]> vboColorArray;
  std::unique_ptr<Vec3[]> vboScalingRotationArray;

  unsigned int vboVertexSize;
  unsigned int vboTextureSize;
  unsigned int vboColorSize;
  unsigned int vboTranslateSize;
  unsigned int vboScalingRotationSize;

  // Each vertex point consists of 2 floats                                [X,Y]
  // (vec2)
  const GLint vertexAttributeSize = 2;
  const GLenum vertexAttributeType = GL_FLOAT;

  // Each texture coordinate for each vertex point consists of 2 floats    [S,T]
  // (vec2)
  const GLint textureAttributeSize = 2;
  const GLenum textureAttributeType = GL_FLOAT;

  // Each Color for each vertex point consists of 4 floats [RGBA], (vec4)
  const GLint colorAttributeSize = 4;
  const GLenum colorAttributeType = GL_FLOAT;

  // Each ScalingRotation for each vertex point consists of 3 floats       [Sx,
  // Sy, rot], (vec3)
  const GLint scalingRotationAttributeSize = 3;
  const GLenum scalingRotationAttributeType = GL_FLOAT;

  // Each ScalingRotation for each vertex point consists of 2 floats       [X,
  // Y], (vec2)
  const GLint translateAttributeSize = 2;
  const GLenum translateAttributeType = GL_FLOAT;
};

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

    Sprite(RenderManager *rm, const std::string &texture, const unsigned int &w,
           const unsigned int &h, const MAP_DEPTH &d,
           const Vec2 &off = Vec2(
               0.0f, 0.0f));  // Have class auto register with a sprite batch;
    ~Sprite();

    Data data;
    bool isActive;
    std::string textureName;
    MAP_DEPTH depth;

    unsigned int textureWidth;
    unsigned int textureHeight;
    float scaleX;
    float scaleY;
    float rotation;
    Vec2 offset;

    RenderSpriteBatch *spriteBatch;
  };

  void Render(const RenderCamera *camera, const RSC_GLProgram *program);

  void AddSprite(Sprite *sprite);
  void DeleteSprite(Sprite *sprite);
  bool CanAddSprites(const int &numSprites);

 protected:
  RenderSpriteBatch(RenderManager *rm, const std::string &tex,
                    const unsigned int &maxSize);

 private:
  VAOWrapperSprite vao;
  const unsigned int maxSprites;
  unsigned int currentSize;

  const std::string textureName;
  const RSC_Texture *texture;

  std::set<Sprite *> sprites;
};

#endif  // LENGINE_RENDERABLE_OBJECT_SPRITE
