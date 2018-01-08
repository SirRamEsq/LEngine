#ifndef L_ENGINE_VECTOR
#define L_ENGINE_VECTOR

#include "math.h"

class Vec4;

class Vec2 {
 public:
  Vec2();
  Vec2(float xVal, float yVal);

  Vec2 Add(Vec2 vec) const;
  Vec2 Subtract(Vec2 vec) const;
  Vec2 Round() const;

  inline void operator=(const Vec2 &v) {
    x = v.x;
    y = v.y;
  }

  inline Vec2 operator+(const Vec2 &v) const {
    Vec2 returnVal;
    returnVal.x = x + v.x;
    returnVal.y = y + v.y;
    return returnVal;
  }

  inline Vec2 operator-(const Vec2 &v) const {
    Vec2 returnVal;
    returnVal.x = x - v.x;
    returnVal.y = y - v.y;
    return returnVal;
  }

  inline Vec2 operator+(float f) const { return Vec2(x + f, y + f); }
  inline Vec2 operator-(float f) const { return Vec2(x - f, y - f); }
  inline void operator=(float f) {
    x = f;
    y = f;
  }
  inline Vec2 operator*(float f) const { return Vec2(x * f, y * f); }
  inline Vec2 Truncate() const { return Vec2(trunc(x), trunc(y)); }

  float x;
  float y;
};

class Vec3 {
 public:
  Vec3();
  Vec3(float xVal, float yVal, float zVal);
  Vec3(const Vec2 &vv, float z);
  Vec3(const Vec4 &vv);

  Vec3 operator+(const Vec3 &rhs);
  Vec3 operator+(float rhs);
  Vec3 &operator+=(const Vec3 &rhs);
  Vec3 operator-(const Vec3 &rhs);
  Vec3 operator-(float rhs);
  Vec3 &operator-=(const Vec3 &rhs);
  Vec3 operator*(float rhs);
  Vec3 &operator*=(float rhs);
  Vec3 operator/(float rhs);
  Vec3 &operator=(const Vec3 &rhs);

  float GetMagnitude();
  Vec3 GetNormalizedVector();
  float DotProduct(const Vec3 &vec);
  Vec3 CrossProduct(const Vec3 &vec);
  float SquaredDistance(Vec3 to);

  Vec3 Add(Vec3 vec);
  Vec3 Subtract(Vec3 vec);
  Vec3 Round();

  float x;
  float y;
  float z;
};

class Vec4 {
 public:
  Vec4();
  Vec4(float xVal, float yVal, float zVal, float wVal);
  Vec4(const Vec2 &v, float zVal, float wVal);
  Vec4(const Vec3 &v, float wVal);

  Vec2 GetVec2() { return Vec2(x, y); }

  float x;
  float y;
  float z;
  float w;
};
#endif
