#ifndef L_POSCOMP
#define L_POSCOMP

#include "../BaseComponent.h"
#include <set>

//Forward-Declare for MapNode
class ComponentPosition;

/**
 * This struct represents the current position of an entity on the map relative to its parent
 */
struct MapNode{
    friend ComponentPosition;

//Functions
    MapNode ();
	~MapNode();

	/**
	 * Update World Position from local coordinates and parent's world coordinates
	 */
	void UpdateWorld();

	/**
	 * Transforms Coordinates relative to this node into World Coordinates
	 * This function should be called by a child node
	 * \param [localCoordinates] The Local Coordinates of the child node
	 */
	Coord2df TranslateLocalToWorld(const Coord2df& localCoordinates);
	/**
	 * Transforms World Coordinates into Coordinates relative to this position
	 * This function should be called by a child node
	 * \param [worldCoordinates] The World Coordinates of the child node
	 */
	Coord2df TranslateWorldToLocal(const Coord2df& worldCoordinates);

	/**
	 * Sets parent and sets Local Coordinates accordingly
	 * without affecting World Coordinates
	 */
	void SetParent(MapNode* parent);

	///Returns the node at the top of the heirarchy (the node without a parent)
	MapNode* GetRootNode();

//Data
	///Pointer to parent
	MapNode* mParent;
	///Local representation of position relative to parent
	Coord2df positionLocal;
	///Absolute representation of position
	Coord2df positionWorld;
};

class ComponentPosition : public BaseComponent{
    public:
        ComponentPosition(EID id, MapNode* parent, const std::string& logFile);
        ~ComponentPosition();

        void Update();
		/// Override 'SetParent', MapNode needs to know when the parent has changed
		void SetParent(BaseComponent* p);

		/// Get Position Relative to Parent
        Coord2df GetPositionLocal();
		/// Get Position Relative to Parent Rounded to an int
		/// \TODO get rid of this function
        Coord2d  GetPositionLocalInt();
		/// Get Absolute Position
        Coord2df GetPositionWorld();
		/// Get Absolute Position Rounded to an int
		/// \TODO get rid of this function
        Coord2d  GetPositionWorldInt();
		/// Get Movement Vector
        Coord2df GetMovement();
		/// Get Acceleration Vector
        Coord2df GetAcceleration();

        void SetPositionLocal   (Coord2df pos);
		/// \TODO get rid of this function
        void SetPositionLocalInt(Coord2d  pos);
        void SetMovement        (Coord2df mov);
        void SetAcceleration    (Coord2df acl);
        void SetMaxSpeed        (float speed ){maximumSpeed=speed;}

		/// \TODO get rid of this function
        void SetPositionLocalX  (float x);
		/// \TODO get rid of this function
        void SetMovementX       (float x);
		/// \TODO get rid of this function
        void SetAccelerationX   (float x);

		/// \TODO get rid of this function
        void SetPositionLocalY  (float y);
		/// \TODO get rid of this function
        void SetMovementY       (float y);
		/// \TODO get rid of this function
        void SetAccelerationY   (float y);

        void IncrementPosition      (Coord2df pos);
        void IncrementMovement      (Coord2df mov);
        void IncrementAcceleration  (Coord2df accel);

        Coord2df TranslateWorldToLocal(const Coord2df& world);
        Coord2df TranslateLocalToWorld(const Coord2df& local);

        MapNode* GetMapNode();

    private:
        Coord2df mMovement;
        Coord2df mAcceleration;
        float maximumSpeed;

        MapNode mNode;
};

class ComponentPositionManager : public BaseComponentManager{
    public:
        ComponentPositionManager(EventDispatcher* e);
        ~ComponentPositionManager();

        void AddComponent(EID id, EID parent=0);

        MapNode* const GetRootNode();

    private:
		/**
		 * This node is used as the root for all other mapNodes
		 * All Position Component nodes are guaranteed to have a parent
		 * if a Position Component doesn't have a parent itself, then the
		 * component's node will have this node as a parent
		 */
        MapNode mRootNode;
};

#endif
