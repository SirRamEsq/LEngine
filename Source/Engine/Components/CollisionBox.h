#ifndef LENGINE_COMP_COLLISION_BOX
#define LENGINE_COMP_COLLISION_BOX

#include "../../Defines.h"
#include "../../Coordinates.h"


enum CB_FLAGS{
    PRIMARY		= 1,
    ENT_CHECK	= 2,
    TILE_CHECK	= 4
};

class ComponentPosition;

class CollisionBox{
	public:
		CollisionBox(unsigned int id, int order, uint8_t flags, std::unique_ptr<Shape>* shape, ComponentPosition* pos);
		bool operator < (const CollisionBox &rhs) const;

		std::unique_ptr<Shape> mShape;

		CollisionResponse Collides(const CollisionBox* box);

	private:
		///Unique identifier for this box
		unsigned int mId;

		uint8_t mFlags;

		///Identifies whether this box is currently active
		bool mActive;

		///Identifies the order in which this box is processed
		int mOrder;

		///Dependency on compPosition... may remove
		ComponentPosition* mPos;
};
#endif
