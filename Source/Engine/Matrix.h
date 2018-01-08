#ifndef L_GL_GLSLHELPER
#define L_GL_GLSLHELPER

#include "Coordinates.h"
#include "Vector.h"
#include "Defines.h"
#include "Errorlog.h"
#include "math.h"

// const used to convert degrees into radians
// is defined in math.h #define M_PI 3.14
#define TAU 2.0 * M_PI
#define ONE_DEG_IN_RAD (2.0 * M_PI) / 360.0  // 0.017444444
#define ONE_RAD_IN_DEG 360.0 / (2.0 * M_PI)  // 57.2957795

#include <GL/gl.h>
#include <GL/glew.h>

/* stored like this:
0  4  8   12
1  5  9   13
2  6  10  14
3  7  11  15
*/
class Matrix4 {
 public:
  Matrix4();

  Matrix4(float a, float b, float c, float d, float e, float f, float g,
          float h, float i, float j, float k, float l, float mm, float n,
          float o, float p);

  Vec4 operator*(const Vec4 &rhs);
  Matrix4 operator*(const Matrix4 &rhs);
  Matrix4 &operator=(const Matrix4 &rhs);

  Matrix4 Translate(const Vec3 &v);
  // Matrix4 RotateX     (const float& deg   );
  // Matrix4 RotateY     (const float& deg   );
  Matrix4 RotateZ(const float &deg);
  Matrix4 Scale(const Vec3 &v);

  float Determinant();
  Matrix4 Inverse();
  Matrix4 Transpose();

  static Matrix4 IdentityMatrix();
  static Matrix4 ZeroMatrix();
  static Matrix4 OrthoGraphicProjectionMatrix(const Vec2 &displaySize);
  static Matrix4 OrthoGraphicProjectionMatrix(const Vec2 &displaySize,
                                              int near, int far);
  static Matrix4 OrthoGraphicProjectionMatrix(const Rect &displayArea);

  GLfloat m[16];
};

#endif  // L_GL_GLSLHELPER
