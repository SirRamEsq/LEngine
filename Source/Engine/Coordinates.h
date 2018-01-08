#ifndef LENGINE_COORDINATES
#define LENGINE_COORDINATES

#include "Defines.h"
#include "Vector.h"

class TiledTileLayer;

inline void CoordToGrid(unsigned int &x, unsigned int &y) {
  x = x / LENGINE_DEF_TILE_W;
  y = y / LENGINE_DEF_TILE_H;
};

inline void GridToCoord(unsigned int &x, unsigned int &y) {
  x = x * LENGINE_DEF_TILE_W;
  y = y * LENGINE_DEF_TILE_H;
};
inline void CoordToGrid(int &x, int &y) {
  x = x / LENGINE_DEF_TILE_W;
  y = y / LENGINE_DEF_TILE_H;
};

inline void GridToCoord(int &x, int &y) {
  x = x * LENGINE_DEF_TILE_W;
  y = y * LENGINE_DEF_TILE_H;
};

struct CollisionResponse {
  CollisionResponse(const Vec2 &vec, bool collided);
  CollisionResponse(float x, float y, bool collided);

  /// The Surface normal of the collisoin
  /// See
  /// https://gamedev.stackexchange.com/questions/136073/how-does-one-calculate-the-surface-normal-in-2d-collisions?s=1|200.4160
  Vec2 mVectorNormal;
  /// Whether a collision exists
  bool mCollided;
};

struct CollisionResponseTile {
  // All tiles collided with
  std::vector<Vec2> collisions;
};

// forward declares
class Circle;
class Rect;
class Shape {
 public:
  // Defines where the shape is considered to originate from
  enum Origin { Left = -1, Center = 0, Right = 1, Top = -1, Bottom = 1 };
  Shape(Vec2 pos);
  Shape(float xx, float yy);

  virtual float GetLeft() const = 0;
  virtual float GetRight() const = 0;
  virtual float GetTop() const = 0;
  virtual float GetBottom() const = 0;

  virtual Vec2 GetCenter() const = 0;

  inline float GetX() const { return x; }
  inline void SetX(const float &xx) { x = xx; }

  inline float GetY() const { return y; }
  inline void SetY(const float &yy) { y = yy; }

  inline Vec2 GetPos() const { return Vec2(x, y); }
  inline void SetPos(const Vec2 pos) {
    x = pos.x;
    y = pos.y;
  }

  void Translate(const Vec2 &pos);

  /// Collision Detection
  virtual CollisionResponse Contains(const Vec2 &point) const = 0;
  /*
   * Used for collision detection
   * this particular virtual function is used to implement a visitor pattern
   */
  virtual CollisionResponse Contains(const Shape *shape) const = 0;
  virtual CollisionResponse Contains(const Rect *r) const = 0;
  virtual CollisionResponse Contains(const Circle *r) const = 0;
  virtual CollisionResponseTile Contains(const TiledTileLayer *layer) const = 0;

  virtual std::unique_ptr<Shape> MakeCopy() const = 0;

  virtual Origin GetOriginHorizontal() const = 0;
  virtual Origin GetOriginVertical() const = 0;

  float x, y;
};

class Rect : public Shape {
 public:
  Rect(float xx, float yy, float ww, float hh);
  Rect(const Vec2 &xy, const Vec2 &wh);
  Rect();

  inline float GetW() const { return w; }
  inline void SetW(const float &ww) { w = ww; }

  inline float GetH() const { return h; }
  inline void SetH(const float &hh) { h = hh; }

  inline Vec2 GetSize() const { return Vec2(w, h); }
  inline void SetSize(Vec2 size) {
    w = size.x;
    h = size.y;
  }

  Rect Round() const;

  float GetTop() const;
  float GetBottom() const;
  float GetLeft() const;
  float GetRight() const;
  Vec2 GetCenter() const;

  CollisionResponse Contains(const Vec2 &point) const;
  /// Visitor
  CollisionResponse Contains(const Shape *shape) const;
  CollisionResponse Contains(const Rect *r) const;
  CollisionResponse Contains(const Circle *r) const;
  CollisionResponseTile Contains(const TiledTileLayer *layer) const;

  Origin GetOriginHorizontal() const;
  Origin GetOriginVertical() const;

  std::unique_ptr<Shape> MakeCopy() const;

  /// Width and height of Rect
  float w, h;
};

class Circle : public Shape {
 public:
  Circle(Vec2 pos, float radius);
  Circle(float xx, float yy, float radius);

  float GetTop() const;
  float GetBottom() const;
  float GetLeft() const;
  float GetRight() const;

  /// Returns position + radius
  Vec2 GetCenter() const;

  inline float GetRadius() const { return r; }
  inline void SetRadius(const float &radius) { r = radius; }

  CollisionResponse Contains(const Vec2 &point) const;
  /// Visitor
  CollisionResponse Contains(const Shape *shape) const;
  CollisionResponse Contains(const Rect *r) const;
  CollisionResponse Contains(const Circle *r) const;
  CollisionResponseTile Contains(const TiledTileLayer *layer) const;

  Origin GetOriginHorizontal() const;
  Origin GetOriginVertical() const;

  std::unique_ptr<Shape> MakeCopy() const;

  /// Radius
  float r;
};

CollisionResponse CollisionRectRect(const Rect &RR, const Rect &R);
CollisionResponse CollisionRectCircle(const Rect &R, const Circle &C);
CollisionResponse CollisionRectPoint(const Rect &R, const Vec2 &point);
CollisionResponse CollisionCircleCircle(const Circle &CC, const Circle &C);
CollisionResponse CollisionCirclePoint(const Circle &CC, const Vec2 &point);

#endif
