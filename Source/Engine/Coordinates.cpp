#include "Coordinates.h"

CollisionResponse::CollisionResponse(const Coord2df& vec, bool collided)
	: mVector(vec), mCollided(collided){

}

CollisionResponse::CollisionResponse(float x, float y, bool collided)
	: mVector(x,y), mCollided(collided){

}
/////////
//Shape//
/////////
Shape::Shape(float xx, float yy)
	: x(xx), y(yy){

}

Shape::Shape(Coord2df pos)
	: x(pos.x), y(pos.y){

}
////////
//Rect//
////////
CRect::CRect(float xx, float yy, float ww, float hh)
	: Shape(xx,yy), w(ww), h(hh){

}
CRect::CRect()
	: Shape(0,0), w(0), h(0){

}

float CRect::GetTop() const{
    if( h > 0 ){ return y; }
    else{ return y + h; }
}
float CRect::GetBottom() const{
    if( h > 0 ){ return y + h; }
    else{ return y; }
}
float CRect::GetLeft() const{
    if( w > 0 ){ return x; }
    else{ return x + w; }
}
float CRect::GetRight() const{
    if( w > 0 ){ return x + w; }
    else{ return x; }
}

CollisionResponse CRect::Contains(const Coord2df& point){
	return CollisionRectPoint(*this, point);
}
CollisionResponse CRect::Contains(const CRect& r){
	return CollisionRectRect(*this,r);
}

CollisionResponse CRect::Contains(const Circle& r){
	return CollisionRectCircle(*this,r);
}

Coord2df CRect::GetCenter() const{
	float left = GetLeft();
	float right = GetRight();
	float xDiff = right - left;

	float top = GetTop();
	float bottom = GetBottom();
	float yDiff = bottom - top;
	return Coord2df(left + (xDiff/2), top + (yDiff/2) );
}

//////////
//Circle//
//////////
Circle::Circle(Coord2df pos, float radius)
	: Shape(pos), r(radius){
	
}

Circle::Circle(float xx, float yy, float radius)
	: Shape(xx, yy), r(radius){
	
}

Coord2df Circle::GetCenter() const{
	return Coord2df(x + r, y + r);
}

float Circle::GetTop     ()  const{
	return y - r;
}

float Circle::GetBottom  ()  const{
	return y + r;
}

float Circle::GetLeft    ()  const{
	return x - r;
}

float Circle::GetRight   ()  const{
	return x + r;
}

CollisionResponse Circle::Contains(const Coord2df& point){
	return CollisionCirclePoint(*this, point);
}
CollisionResponse Circle::Contains(const CRect& r){
	return CollisionRectCircle(r, *this);
}
CollisionResponse Circle::Contains(const Circle& r){
	return CollisionCircleCircle(*this, r);
}

/////////////
//Collision//
/////////////
CollisionResponse CollisionRectRect(const CRect& RR, const CRect& R){
    if (R.GetBottom() < RR.GetTop())   {return CollisionResponse(0,0,false);}
    if (R.GetTop()    > RR.GetBottom()){return CollisionResponse(0,0,false);}
    if (R.GetRight()  < RR.GetLeft())  {return CollisionResponse(0,0,false);}
    if (R.GetLeft()   > RR.GetRight()) {return CollisionResponse(0,0,false);}

	return CollisionResponse(RR.GetCenter() - R.GetCenter(),true);
}

CollisionResponse CollisionRectCircle(const CRect& R, const Circle C){
    if (R.GetBottom() < C.GetTop())   {return CollisionResponse(0,0,false);}
    if (R.GetTop()    > C.GetBottom()){return CollisionResponse(0,0,false);}
    if (R.GetRight()  < C.GetLeft())  {return CollisionResponse(0,0,false);}
    if (R.GetLeft()   > C.GetRight()) {return CollisionResponse(0,0,false);}

	return CollisionResponse(0,0,false);
}

CollisionResponse CollisionRectPoint(const CRect& R, const Coord2df& point){
    int leftR, rightR, topR, bottomR;

    leftR= R.GetLeft();
    topR= R.GetTop();
    rightR= R.GetRight();
    bottomR= R.GetBottom();

    if(((point.x <= rightR) && (point.x >= leftR))){ //check for a horizontal collision
        if(((point.y >= topR) && (point.y <= bottomR))){//check for a vertical collision
            return CollisionResponse(R.GetCenter() - point, true);
        }
    }
	return CollisionResponse(0,0,false);
}

CollisionResponse CollisionCircleCircle(const Circle& CC, const Circle& C){
	return CollisionResponse(0,0,false);
}

CollisionResponse CollisionCirclePoint(const Circle& CC, const Coord2df& point){
	return CollisionResponse(0,0,false);

}

