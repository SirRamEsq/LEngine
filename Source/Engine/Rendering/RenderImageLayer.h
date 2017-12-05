#ifndef LENGINE_RENDERABLE_IMAGE
#define LENGINE_RENDERABLE_IMAGE

#include "../Resources/RSC_Map.h"
#include "../glslHelper.h"
#include "RenderableObject.h"
#include "VAOWrapper.h"

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

  bool isTransparent();

  const TiledImageLayer *layer;

 private:
  // unsigned int animatedRefreshRateTimer;
  // bool animated;

  VAOWrapper2D vao;
};

#endif  // LENGINE_RENDERABLE_IMAGE
