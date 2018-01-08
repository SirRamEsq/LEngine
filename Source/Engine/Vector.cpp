#include "Vector.h"

////////
// Vec2//
////////

Vec2::Vec2() {
  x = 0.0f;
  y = 0.0f;
}
Vec2::Vec2(float xVal, float yVal) : x(xVal), y(yVal) {}

Vec2 Vec2::Add(Vec2 v) const { return Vec2(x + v.x, y + v.y); }
Vec2 Vec2::Subtract(Vec2 v) const { return Vec2(x - v.x, y - v.y); }
Vec2 Vec2::Round() const { return Vec2(floor(x + 0.5), floor(y + 0.5)); }

////////
// Vec3//
////////

Vec3::Vec3() {
  x = 0.0f;
  y = 0.0f;
  z = 0.0f;
}
Vec3::Vec3(float xVal, float yVal, float zVal) : x(xVal), y(yVal), z(zVal) {}
Vec3::Vec3(const Vec2 &vv, float zVal) {
  x = vv.x;
  y = vv.y;
  z = zVal;
}

Vec3::Vec3(const Vec4 &vv) {
  x = vv.x;
  y = vv.y;
  z = vv.z;
}
Vec3 Vec3::Add(Vec3 v) { return Vec3(x + v.x, y + v.y, z + v.z); }
Vec3 Vec3::Subtract(Vec3 v) { return Vec3(x - v.x, y - v.y, z - v.z); }
Vec3 Vec3::Round() {
  return Vec3(floor(x + 0.5), floor(y + 0.5), floor(z + 0.5));
}

float Vec3::GetMagnitude() { return sqrt((x * x) + (y * y) + (z * z)); }

Vec3 Vec3::GetNormalizedVector() {
  float mag = GetMagnitude();
  if (0.0f == mag) {
    return Vec3(0.0f, 0.0f, 0.0f);
  }
  Vec3 vb;
  vb.x = x / mag;
  vb.y = y / mag;
  vb.z = z / mag;

  return vb;
}

float Vec3::DotProduct(const Vec3 &vec) {
  return (x * vec.x) + (y * vec.y) + (z * vec.z);
}

Vec3 Vec3::CrossProduct(const Vec3 &vec) {
  float x = y * vec.z - z * vec.y;
  float y = z * vec.x - x * vec.z;
  float z = x * vec.y - y * vec.x;
  return Vec3(x, y, z);
}

float Vec3::SquaredDistance(Vec3 to) {
  float x = (to.x - x) * (to.x - x);
  float y = (to.y - y) * (to.y - y);
  float z = (to.z - z) * (to.z - z);
  return x + y + z;
}

Vec3 Vec3::operator+(const Vec3 &rhs) {
  Vec3 vc;
  vc.x = x + rhs.x;
  vc.y = y + rhs.y;
  vc.z = z + rhs.z;
  return vc;
}

Vec3 &Vec3::operator+=(const Vec3 &rhs) {
  x += rhs.x;
  y += rhs.y;
  z += rhs.z;
  return *this;  // return self
}

Vec3 Vec3::operator-(const Vec3 &rhs) {
  Vec3 vc;
  vc.x = x - rhs.x;
  vc.y = y - rhs.y;
  vc.z = z - rhs.z;
  return vc;
}

Vec3 &Vec3::operator-=(const Vec3 &rhs) {
  x -= rhs.x;
  y -= rhs.y;
  z -= rhs.z;
  return *this;
}

Vec3 Vec3::operator+(float rhs) {
  Vec3 vc;
  vc.x = x + rhs;
  vc.y = y + rhs;
  vc.z = z + rhs;
  return vc;
}

Vec3 Vec3::operator-(float rhs) {
  Vec3 vc;
  vc.x = x - rhs;
  vc.y = y - rhs;
  vc.z = z - rhs;
  return vc;
}

Vec3 Vec3::operator*(float rhs) {
  Vec3 vc;
  vc.x = x * rhs;
  vc.y = y * rhs;
  vc.z = z * rhs;
  return vc;
}

Vec3 Vec3::operator/(float rhs) {
  Vec3 vc;
  vc.x = x / rhs;
  vc.y = y / rhs;
  vc.z = z / rhs;
  return vc;
}

Vec3 &Vec3::operator*=(float rhs) {
  x = x * rhs;
  y = y * rhs;
  z = z * rhs;
  return *this;
}

Vec3 &Vec3::operator=(const Vec3 &rhs) {
  x = rhs.x;
  y = rhs.y;
  z = rhs.z;
  return *this;
}

////////
// Vec4//
////////

Vec4::Vec4() {}

Vec4::Vec4(float xVal, float yVal, float zVal, float wVal) {
  x = xVal;
  y = yVal;
  z = zVal;
  w = wVal;
}

Vec4::Vec4(const Vec2 &vv, float zVal, float wVal) {
  x = vv.x;
  y = vv.y;
  z = zVal;
  w = wVal;
}

Vec4::Vec4(const Vec3 &vv, float wVal) {
  x = vv.x;
  y = vv.y;
  z = vv.z;
  w = wVal;
}
