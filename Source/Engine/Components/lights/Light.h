#ifndef LENGINE_LIGHT
#define LENGINE_LIGHT

#include "../../glslHelper.h"
#include "../../Resources/RSC_GLShader.h"

/// POD class
class Light {
 public:
  /// RGB and intensity
  Vec4 color;

  /// Light flickering value [0.0f - 1.0f]
  float noise;

  /// Whether or not light is rendered
  bool render;

  RSC_GLProgram* shaderProgram;
};

#endif
