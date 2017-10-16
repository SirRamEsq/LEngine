#include "CollisionBox.h"
#include "../CompPosition.h"

CollisionBox::CollisionBox(unsigned int id, int order, uint8_t flags, std::unique_ptr<Shape>* shape, ComponentPosition* pos)
	: mId(id), mOrder(order), mFlags(flags), mShape(std::move(*shape)), mPos(pos){

	ASSERT( mShape.get() != NULL);
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

CollisionResponse CollisionBox::Collides(const CollisionBox* box){
	auto translationValue1 = mPos->GetPositionWorld();
	auto translationValue2 = box->mPos->GetPositionWorld();

	this-> mShape->Translate(translationValue1);
	box->  mShape->Translate(translationValue2);

	auto result = mShape->Contains(box->mShape.get());

	this-> mShape->Translate(translationValue1 * -1.0f);
	box->  mShape->Translate(translationValue2 * -1.0f);

	return result;
}

