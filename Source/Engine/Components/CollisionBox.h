#ifndef LENGINE_COMP_COLLISION_BOX
#define LENGINE_COMP_COLLISION_BOX

#include "../Defines.h"
#include "../Coordinates.h"


enum CB_FLAGS{
    PRIMARY		= 1,
    ENT_CHECK	= 2,
    TILE_CHECK	= 4
};

class ComponentPosition;

class CollisionBox{
	public:
		CollisionBox(unsigned int id, int order, uint8_t flags, const Shape& shape, ComponentPosition* pos);
		bool operator < (const CollisionBox &rhs) const;
		CollisionResponse Collides(const CollisionBox* box);

		void Activate();
		void Deactivate();
		bool IsActive() const;

		void SetShape(const Shape* shape);
		void UpdateWorldCoord();
		const Shape* GetWorldCoord();

		void SetFlags(uint8_t flags);
		uint8_t Flags();

		unsigned int GetId();

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

		std::unique_ptr<Shape> mShape;
		std::unique_ptr<Shape> mShapeToWorldCoords;
};
#endif
