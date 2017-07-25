#include "CompPosition.h"
#include "math.h"
#include "../Errorlog.h"

///////////
//MapNode//
///////////
MapNode::MapNode(){
	mParent = NULL;
}
MapNode::~MapNode(){
}

void MapNode::SetParent(MapNode* parent){
	if(mParent == NULL){
		mParent = parent;
		if(parent == NULL){return;}

		positionLocal = mParent->TranslateWorldToLocal(positionLocal);
	}
	Coord2df oldParentCoord, newParentcoord;
	//Assume no parent is set and set local coordinates to world
	positionLocal = mParent->TranslateLocalToWorld(positionLocal);

	mParent = parent;
	if(parent==NULL){return;}

	//Convert world coordinates back to local
	//This time local being relative to the new parent
	positionLocal = mParent->TranslateWorldToLocal(positionLocal);
}

void MapNode::UpdateWorld(){
	//Component manager will ensure that the parents are run before the children
	//This guarantees that the parent's world position is up-to-date
	
	//Set World Coordinates to Local Coordinates
	positionWorld = positionLocal;

	//If this node has a parent, translate the world Coordinates by their world coordinates
	if(mParent!=NULL){
		positionWorld = positionWorld + mParent->positionWorld;
	}
}

MapNode* MapNode::GetRootNode(){
	//If this node doesn't have a parent, it is the root
	if(mParent==NULL){return this;}

	//Recurse until Root is found
	return mParent->GetRootNode();
}

Coord2df MapNode::TranslateLocalToWorld(const Coord2df& localCoordinates){
	/// \TODO Setup unit test 
	Coord2df worldCoordinates(localCoordinates);
	if(mParent==NULL){return worldCoordinates;}

	//Translate coordinates by the local position of this node
	worldCoordinates = localCoordinates + positionLocal;

	//Translate coordinates by the local position of the parent node
	return mParent->TranslateLocalToWorld(worldCoordinates);
}

Coord2df MapNode::TranslateWorldToLocal(const Coord2df& worldCoordinates){
	/// \TODO Setup unit test
	Coord2df localCoordinates(worldCoordinates);
	if(mParent==NULL){return localCoordinates;}

	//Translate coordinates by the local position of this node
	localCoordinates=worldCoordinates - positionLocal;

	//Translate coordinates by the local position of the parent node
	return mParent->TranslateWorldToLocal(localCoordinates);
}

/////////////////////
//ComponentPosition//
/////////////////////
ComponentPosition::ComponentPosition(EID id, MapNode* parent, const std::string& logName) : BaseComponent(id, logName){
	mEntityID=id;
	mNode.SetParent(parent);
	maximumSpeed = 15.0;
}
ComponentPosition::~ComponentPosition(){
}

void ComponentPosition::SetParent(BaseComponent* p){
	//If parent is null, then set the node parent to the root node owned by the manager
	if(p==NULL){
		mNode.SetParent(mNode.GetRootNode());
		return;
	}
	//Set node's parent to the parent's node
	else{
		auto parentPosition = static_cast<ComponentPosition*>(p);
		mNode.SetParent(parentPosition->GetMapNode());
	}

	//Set this component's parent to the parent
	parent = p;
}

Coord2df ComponentPosition::GetPositionWorld(){
	return mNode.positionWorld;
}
Coord2d ComponentPosition::GetPositionWorldInt(){
	return Coord2d( floor(mNode.positionWorld.x + 0.5f), floor(mNode.positionWorld.y + 0.5f) );
}
Coord2df ComponentPosition::GetPositionLocal(){
	return mNode.positionLocal;
}
Coord2d ComponentPosition::GetPositionLocalInt(){
	return Coord2d( floor(mNode.positionLocal.x + 0.5f), floor(mNode.positionLocal.y + 0.5f) );
}

Coord2df ComponentPosition::GetMovement(){
	return mMovement;
}

Coord2df ComponentPosition::GetAcceleration(){
	return mAcceleration;
}

void ComponentPosition::Update(){
	//Clamp movement speed to maximum
	if		(mMovement.x > maximumSpeed ){mMovement.x =  maximumSpeed;}
	else if (mMovement.x < -maximumSpeed){mMovement.x = -maximumSpeed;}
	if		(mMovement.y > maximumSpeed ){mMovement.y =  maximumSpeed;}
	else if (mMovement.y < -maximumSpeed){mMovement.y = -maximumSpeed;}

	//Increment local position by movement speed
	mNode.positionLocal= mNode.positionLocal + mMovement;

	//Increment movement by acceleration
	mMovement= mMovement + mAcceleration;

	mNode.UpdateWorld();
}

void ComponentPosition::IncrementPosition(Coord2df pos){
	mNode.positionLocal=mNode.positionLocal + pos;
	mNode.positionWorld=mNode.positionWorld + pos;
}
void ComponentPosition::IncrementMovement(Coord2df mov){
	mMovement= mMovement+mov;
}
void ComponentPosition::IncrementAcceleration(Coord2df accel){
	mAcceleration=mAcceleration + accel;
}

void ComponentPosition::SetPositionLocal	(Coord2df pos){
	mNode.positionLocal	 = pos;
	mNode.UpdateWorld();
}
void ComponentPosition::SetPositionLocalInt    (Coord2d pos){
	SetPositionLocal(Coord2df(pos.x, pos.y));
}
void ComponentPosition::SetMovement			(Coord2df mov){mMovement=mov;}
void ComponentPosition::SetAcceleration		(Coord2df acl){mAcceleration=acl;}

void ComponentPosition::SetPositionLocalX	(float x){
	SetPositionLocal(Coord2df(x, mNode.positionLocal.y));
}
void ComponentPosition::SetMovementX		(float x){mMovement.x=x;}
void ComponentPosition::SetAccelerationX	(float x){mAcceleration.x=x;}

void ComponentPosition::SetPositionLocalY	(float y){
	SetPositionLocal(Coord2df(mNode.positionLocal.x, y));
}
void ComponentPosition::SetMovementY		(float y){mMovement.y=y;}
void ComponentPosition::SetAccelerationY	(float y){mAcceleration.y=y;}

Coord2df ComponentPosition::TranslateWorldToLocal(const Coord2df& world){
	//Node is guarnteed to have a parent, if the component doesn't have a parent
	//then the node's parent is set to the rootNode owned by the Manager
	return mNode.mParent->TranslateWorldToLocal(world);
}

Coord2df ComponentPosition::TranslateLocalToWorld(const Coord2df& local){
	//Node is guarnteed to have a parent, if the component doesn't have a parent
	//then the node's parent is set to the rootNode owned by the Manager
	return mNode.mParent->TranslateLocalToWorld(local);
}

MapNode* ComponentPosition::GetMapNode(){
	return &mNode;
}

////////////////////////////
//ComponentPositionManager//
////////////////////////////
ComponentPositionManager::ComponentPositionManager(EventDispatcher* e) : BaseComponentManager("LOG_COMP_POSITION", e){
}

ComponentPositionManager::~ComponentPositionManager(){
}

void ComponentPositionManager::AddComponent(EID id, EID parent){
	//Return if component with EID exists
	auto i = componentList.find(id);
	if(i != componentList.end()){return;}

	//Assign manager's root node as the node's parent by default
	ComponentPosition* pos=new ComponentPosition(id, &mRootNode, logFileName);
	pos->mManager=this;
	componentList[id]=pos;

	//Change component's parent
	//If the parent is valid, its map node will become the new node's parent
	SetParent(pos->GetEID(), parent);
}

MapNode* const ComponentPositionManager::GetRootNode(){
	return &mRootNode;
}
