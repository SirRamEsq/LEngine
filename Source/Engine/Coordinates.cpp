#include "Coordinates.h"

#include "Resources/RSC_Map.h"
#include <memory>

CollisionResponse::CollisionResponse(const Coord2df &vec, bool collided)
    : mVectorNormal(vec), mCollided(collided) {}

CollisionResponse::CollisionResponse(float x, float y, bool collided)
    : mVectorNormal(x, y), mCollided(collided) {}
/////////
// Shape//
/////////
Shape::Shape(float xx, float yy) : x(xx), y(yy) {}

Shape::Shape(Coord2df pos) : x(pos.x), y(pos.y) {}

void Shape::Translate(const Coord2df &pos) {
  x += pos.x;
  y += pos.y;
}

////////
// Rect//
////////
Rect::Rect(float xx, float yy, float ww, float hh)
    : Shape(xx, yy), w(ww), h(hh) {}
Rect::Rect(const Coord2df &xy, const Coord2df &wh)
    : Shape(xy), w(wh.x), h(wh.y) {}
Rect::Rect() : Shape(0, 0), w(0), h(0) {}

Rect Rect::Round() const {
  auto xy = Coord2df(x, y);
  auto wh = Coord2df(w, h);
  xy = xy.Round();
  wh = wh.Round();
  return Rect(xy, wh);
}

std::unique_ptr<Shape> Rect::MakeCopy() const {
  auto returnValue = std::make_unique<Rect>(x, y, w, h);
  return std::move(returnValue);
}

Shape::Origin Rect::GetOriginHorizontal() const {
  if (w >= 0) {
    return Shape::Origin::Left;
  }
  return Shape::Origin::Right;
}

Shape::Origin Rect::GetOriginVertical() const {
  if (h >= 0) {
    return Shape::Origin::Top;
  }
  return Shape::Origin::Bottom;
}

float Rect::GetTop() const {
  if (h > 0) {
    return y;
  } else {
    return y + h;
  }
}
float Rect::GetBottom() const {
  if (h > 0) {
    return y + h;
  } else {
    return y;
  }
}
float Rect::GetLeft() const {
  if (w > 0) {
    return x;
  } else {
    return x + w;
  }
}
float Rect::GetRight() const {
  if (w > 0) {
    return x + w;
  } else {
    return x;
  }
}

CollisionResponse Rect::Contains(const Coord2df &point) const {
  return CollisionRectPoint(*this, point);
}
CollisionResponse Rect::Contains(const Rect *r) const {
  return CollisionRectRect(*this, *r);
}

CollisionResponse Rect::Contains(const Circle *r) const {
  return CollisionRectCircle(*this, *r);
}

Coord2df Rect::GetCenter() const {
  float left = GetLeft();
  float right = GetRight();
  float xDiff = right - left;

  float top = GetTop();
  float bottom = GetBottom();
  float yDiff = bottom - top;
  return Coord2df(left + (xDiff / 2), top + (yDiff / 2));
}

CollisionResponse Rect::Contains(const Shape *shape) const {
  return shape->Contains(this);
}

CollisionResponseTile Rect::Contains(const TiledTileLayer *layer) const {
  CollisionResponseTile returnValue;
  if (layer == NULL) {
    return returnValue;
  }
  auto collisions = &returnValue.collisions;
  Coord2df ul(0, 0), dr(0, 0);

  ul.x = GetLeft();
  ul.y = GetTop();
  dr.x = GetRight();
  dr.y = GetBottom();
  int txx1 = ul.x;
  int txx2 = dr.x;
  int tyy1 = ul.y;
  int tyy2 = dr.y;

  CoordToGrid(txx1, tyy1);
  CoordToGrid(txx2, tyy2);
  if ((txx1 == txx2) and
      (tyy1 == tyy2)) {  // if the top left is the same as the bottom right,
    if (layer->HasTile(txx1, tyy1)) {
      collisions->push_back(Coord2df(txx1, tyy1));
    }
    return returnValue;
  }

  int differenceX = txx2 - txx1;  // Both differences will always be positive
  int differenceY = tyy2 - tyy1;

  bool negativeH = (GetOriginHorizontal() == Shape::Origin::Right);
  bool negativeW = (GetOriginVertical() == Shape::Origin::Bottom);
  int tx, ty;

  if (negativeW) {
    tx = txx2;
  } else {
    tx = txx1;
  }
  if (negativeH) {
    ty = tyy2;
  } else {
    ty = tyy1;
  }

  for (int iter = 0; iter <= differenceX; iter++) {
    for (int iter2 = 0; iter2 <= differenceY; iter2++) {
      if (layer->HasTile(tx, ty)) {
        collisions->push_back(Coord2df(tx, ty));
      }
      if (!negativeH) {
        ty += 1;
      } else {
        ty -= 1;
      }
    }
    if (negativeH) {
      ty = tyy2;
    } else {
      ty = tyy1;
    }
    if (!negativeW) {
      tx += 1;
    } else {
      tx -= 1;
    }
  }
  return returnValue;
}

//////////
// Circle//
//////////
Circle::Circle(Coord2df pos, float radius) : Shape(pos), r(radius) {}

Circle::Circle(float xx, float yy, float radius) : Shape(xx, yy), r(radius) {}

Shape::Origin Circle::GetOriginHorizontal() const {
  return Shape::Origin::Center;
}

Shape::Origin Circle::GetOriginVertical() const {
  return Shape::Origin::Center;
}

std::unique_ptr<Shape> Circle::MakeCopy() const {
  auto returnValue = std::make_unique<Circle>(x, y, r);
  return std::move(returnValue);
}

Coord2df Circle::GetCenter() const { return Coord2df(x + r, y + r); }

float Circle::GetTop() const { return y - r; }

float Circle::GetBottom() const { return y + r; }

float Circle::GetLeft() const { return x - r; }

float Circle::GetRight() const { return x + r; }

CollisionResponse Circle::Contains(const Coord2df &point) const {
  return CollisionCirclePoint(*this, point);
}
CollisionResponse Circle::Contains(const Rect *r) const {
  return CollisionRectCircle(*r, *this);
}
CollisionResponse Circle::Contains(const Circle *r) const {
  return CollisionCircleCircle(*this, *r);
}

CollisionResponse Circle::Contains(const Shape *shape) const {
  return shape->Contains(this);
}
CollisionResponseTile Circle::Contains(const TiledTileLayer *layer) const {
  return CollisionResponseTile();
}
/////////////
// Collision//
/////////////
CollisionResponse CollisionRectRect(const Rect &RR, const Rect &R) {
  if (R.GetBottom() < RR.GetTop()) {
    return CollisionResponse(0, 0, false);
  }
  if (R.GetTop() > RR.GetBottom()) {
    return CollisionResponse(0, 0, false);
  }
  if (R.GetRight() < RR.GetLeft()) {
    return CollisionResponse(0, 0, false);
  }
  if (R.GetLeft() > RR.GetRight()) {
    return CollisionResponse(0, 0, false);
  }

  return CollisionResponse(RR.GetCenter() - R.GetCenter(), true);
}

CollisionResponse CollisionRectCircle(const Rect &R, const Circle &C) {
  if (R.GetBottom() < C.GetTop()) {
    return CollisionResponse(0, 0, false);
  }
  if (R.GetTop() > C.GetBottom()) {
    return CollisionResponse(0, 0, false);
  }
  if (R.GetRight() < C.GetLeft()) {
    return CollisionResponse(0, 0, false);
  }
  if (R.GetLeft() > C.GetRight()) {
    return CollisionResponse(0, 0, false);
  }

  return CollisionResponse(0, 0, false);
}

CollisionResponse CollisionRectPoint(const Rect &R, const Coord2df &point) {
  int leftR, rightR, topR, bottomR;

  leftR = R.GetLeft();
  topR = R.GetTop();
  rightR = R.GetRight();
  bottomR = R.GetBottom();

  if (((point.x <= rightR) &&
       (point.x >= leftR))) {  // check for a horizontal collision
    if (((point.y >= topR) &&
         (point.y <= bottomR))) {  // check for a vertical collision
      return CollisionResponse(R.GetCenter() - point, true);
    }
  }
  return CollisionResponse(0, 0, false);
}

CollisionResponse CollisionCircleCircle(const Circle &CC, const Circle &C) {
  return CollisionResponse(0, 0, false);
}

CollisionResponse CollisionCirclePoint(const Circle &CC,
                                       const Coord2df &point) {
  return CollisionResponse(0, 0, false);
}
