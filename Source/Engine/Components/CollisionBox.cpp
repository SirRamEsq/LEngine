#include "CollisionBox.h"
#include "CompPosition.h"

CollisionBox::CollisionBox(int order, uint8_t flags, const Shape* shape, ComponentPosition* pos)
	: mOrder(order), mFlags(flags), mPos(pos){

	mActive = true;
	ASSERT( mShape.get() != NULL);

	SetShape(shape);
}

void CollisionBox::SetShape(const Shape* shape){
	mShape.reset(shape->MakeCopy().release());
	mShapeToWorldCoords.reset(shape->MakeCopy().release());
}

bool CollisionBox::operator < (const CollisionBox &rhs) const{
    if( ((mFlags&PRIMARY)==PRIMARY)and((rhs.mFlags&PRIMARY)==PRIMARY) ){
            if(mOrder>=rhs.mOrder)  {return true;}
            else                    {return false;}
    }
    if((mFlags&PRIMARY)==PRIMARY){return true;}
    if((rhs.mFlags&PRIMARY)==PRIMARY){return false;}

    if(mOrder>=rhs.mOrder)  {return true;}
    else                    {return false;}

    return true;
}

const Shape* CollisionBox::GetWorldCoord(){
	return mShapeToWorldCoords.get();
}

void CollisionBox::UpdateWorldCoord(){
	auto wPos = mPos->GetPositionWorld();
	mShapeToWorldCoords->x = mShape->x + wPos.x;
	mShapeToWorldCoords->y = mShape->y + wPos.y;
}

CollisionResponse CollisionBox::Collides(const CollisionBox* box){
	auto result = mShapeToWorldCoords->Contains(box->mShapeToWorldCoords.get());
	return result;
}

void CollisionBox::Activate(){
	mActive = true;
}

void CollisionBox::Deactivate(){
	mActive = false;
}

bool CollisionBox::IsActive() const{
	return mActive;
}

void CollisionBox::SetFlags(uint8_t flags){
	mFlags = flags;
}

uint8_t CollisionBox::Flags(){
	return mFlags;
}

