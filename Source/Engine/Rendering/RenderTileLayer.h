#ifndef LENGINE_RENDERABLE_OBJECT_TLAYER
#define LENGINE_RENDERABLE_OBJECT_TLAYER

#include "../Resources/RSC_Map.h"
#include "../glslHelper.h"
#include "RenderableObject.h"
#include "VAOWrapper.h"

class RenderTileLayer : public RenderableObjectWorld {
 public:
  RenderTileLayer(RenderManager *rm, const TiledTileLayer *l);
  ~RenderTileLayer();

  void BuildVAO();
  void BuildVAOArea(Rect area);

  void Render(const RenderCamera *camera, const RSC_GLProgram *program);
  const TiledTileLayer *layer;

  bool isTransparent();

 private:
  /// Returns true if any data was updated
  bool UpdateTileAnimations(const RenderCamera *camera);
  /// Get the VAO vertex index from a texture coordinate
  unsigned int CalculateVertexIndex(unsigned int x, unsigned int y);
  /// Sets the texture of a tile @ vertexIndex to the image associated with the
  /// passed gid
  void SetTileTexture(unsigned int vertexIndex, GID gid);
  void BuildVAOTile(unsigned int x, unsigned int y);

  unsigned int animatedRefreshRateTimer;
  bool animated;

  const TiledSet *tiledSet;
  VAOWrapper2D vao;

  int textureWidth;
  int textureHeight;
};

#endif  // LENGINE_RENDERABLE_OBJECT_TLAYER
