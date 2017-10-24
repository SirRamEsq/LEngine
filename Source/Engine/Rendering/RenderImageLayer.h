#ifndef LENGINE_RENDERABLE_IMAGE
#define LENGINE_RENDERABLE_IMAGE

#include "../Resources/RSC_Map.h"
#include "../glslHelper.h"
#include "RenderableObject.h"

class VAOWrapper {
 public:
  // Max size is the number of objects, not the number of verticies.
  // VAOWrapper will figure out the number of vertiices from the number of
  // objects
  VAOWrapper(const unsigned int &maxSize);
  ~VAOWrapper();
  void UpdateGPU();

  Vec2 *GetVertexArray() { return vboVertexArray.get(); }
  Vec2 *GetTextureArray() { return vboTextureArray.get(); }
  Vec4 *GetColorArray() { return vboColorArray.get(); }

  GLuint GetVAOID() { return vao; }

 private:
  GLuint vboVertex;
  GLuint vboTexture;
  GLuint vboColor;
  GLuint vao;

  std::unique_ptr<Vec2[]> vboVertexArray;
  std::unique_ptr<Vec2[]> vboTextureArray;
  std::unique_ptr<Vec4[]> vboColorArray;

  unsigned int vboVertexSize;
  unsigned int vboTextureSize;
  unsigned int vboColorSize;

  const unsigned int vboMaxSize;

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
};

class RenderImageLayer : public RenderableObjectWorld {
 public:
  struct RenderableBufferData {
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
  };

  RenderImageLayer(RenderManager *rm, TiledImageLayer *l);
  ~RenderImageLayer();

  void BuildVAO(Rect camera);

  void Render(const RenderCamera *camera, const RSC_GLProgram *program);

  const TiledImageLayer *layer;

 private:
  // unsigned int animatedRefreshRateTimer;
  // bool animated;

  VAOWrapper vao;
};

#endif  // LENGINE_RENDERABLE_IMAGE
