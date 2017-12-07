#ifndef LENGINE_LIGHT_POINT
#define LENGINE_LIGHT_POINT

#include "Light.h"

class PointLight : public Light {
 public:
  PointLight() { type = LIGHT_POINT; }
};

#endif
