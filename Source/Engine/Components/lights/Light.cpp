#include "Light.h"

Light::Light() {
  color.x = 1.0f;
  color.y = 1.0f;
  color.z = 1.0f;
  pos.x= 0.0;
  pos.y= 0.0;
  pos.z= 0.0;
  worldPos.x= 0.0;
  worldPos.y= 0.0;
  noise = 0;
  distance = 100;
  render = true;
}
