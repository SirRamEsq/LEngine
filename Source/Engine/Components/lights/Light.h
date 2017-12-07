#ifndef LENGINE_LIGHT
#define LENGINE_LIGHT

#include "../../glslHelper.h"
#include "../../Resources/RSC_GLShader.h"

enum LIGHT_TYPE { LIGHT_GENERIC = 0, LIGHT_POINT = 1 };

/// POD class
class Light {
 public:
  /// RGB
  Vec3 color;

  /// X,Y,Depth in world coorindates
  Vec3 pos;

  /// Light flickering value [0.0f - 1.0f]
  float noise;

  /// Distance the light extends to
  float distance;

  /// Whether or not light is rendered
  bool render;

  LIGHT_TYPE type;
};

#endif
