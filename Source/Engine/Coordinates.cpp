#include "Coordinates.h"

CollisionResponse::CollisionResponse(const Coord2df& vec, bool collided)
	: mVector(vec), mCollided(collided){

}

CollisionResponse::CollisionResponse(float x, float y, bool collided)
	: mVector(x,y), mCollided(collided){

}

int CRect::GetTop() const{
    if(h>0){return y;}
    else{return y+h;}
}
int CRect::GetBottom() const{
    if(h>0){return y+h;}
    else{return y;}
}
int CRect::GetLeft() const{
    if(w>0){return x;}
    else{return x+w;}
}
int CRect::GetRight() const{
    if(w>0){return x+w;}
    else{return x;}
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



float FloatRect::GetTop() const{
    if(h>0){return y;}
    else{return y+h;}
}
float FloatRect::GetBottom() const{
    if(h>0){return y+h;}
    else{return y;}
}
float FloatRect::GetLeft() const{
    if(w>0){return x;}
    else{return x+w;}
}
float FloatRect::GetRight() const{
    if(w>0){return x+w;}
    else{return x;}
}



Circle::Circle(Coord2df pos, float radius)
	: mPos(pos), mRadius(radius){
	
}

Circle::Circle(float x, float y, float radius)
	: mPos(x, y), mRadius(radius){
	
}

float Circle::GetTop     ()  const{
	return mPos.y - mRadius;
}

float Circle::GetBottom  ()  const{
	return mPos.y + mRadius;
}

float Circle::GetLeft    ()  const{
	return mPos.x - mRadius;
}

float Circle::GetRight   ()  const{
	return mPos.x + mRadius;
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

