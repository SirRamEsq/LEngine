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
  void BuildVAOTile(unsigned int x, unsigned int y);

  unsigned int animatedRefreshRateTimer;
  bool animated;

  const TiledSet *tiledSet;
  VAOWrapper2D vao;

  int textureWidth;
  int textureHeight;
};

#endif  // LENGINE_RENDERABLE_OBJECT_TLAYER
