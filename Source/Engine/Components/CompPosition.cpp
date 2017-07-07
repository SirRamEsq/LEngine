#include "CompPosition.h"
#include "math.h"
#include "../Errorlog.h"

MapNode::MapNode(MapNode* parent){
	mDirty=true;
	mParent=parent;
	if(mParent!=NULL){
		mParent->AddChild(this);
	}
}
MapNode::~MapNode(){
	if(mParent!=NULL){
		mParent->RemoveChild(this);
	}

	for(tNodeSet::iterator i=mChildren.begin(); i!=mChildren.end(); ){
		(*i)->ChangeParent(mParent);
		i=mChildren.begin();
	}
}

void MapNode::ChangeParent(MapNode* parent){
	Coord2df oldParentCoord, newParentcoord;

	mParent->TranslateLocalToWorld(mPositionLocal);

	mParent->RemoveChild(this);
	mParent = parent;
	if(parent!=NULL){mParent->AddChild(this);}
	else{return;}

	//Need to get the world coordinates
	oldParentCoord=mParent->mPositionWorld;
	newParentcoord=parent->mPositionWorld;

	//Add the Old parent coordinates to get the proper World Coordinates
	//Then subtract that from the new parent to get the proper Local Coordinates
	mParent->TranslateWorldToLocal(mPositionLocal);
}

void MapNode::UpdateWorld(const Coord2df& worldCoordinates, bool dirty){
	mDirty = (mDirty | dirty); //if either this node or it's parent are dirty

	//Actually using the dirty flag is probably slower than just adding the coordinates together anyway; branch prediction n'at
	//if(mDirty){
	mPositionWorld=mPositionLocal + worldCoordinates;
	//}

	tNodeSet::iterator i=mChildren.begin();
	for(; i!=mChildren.end(); i++){
		(*i)->UpdateWorld(mPositionWorld, mDirty);
	}

	mDirty=false;
}

MapNode* MapNode::GetRootNode(){
	if(mParent==NULL){return this;}
	return mParent->GetRootNode();
}

void MapNode::TranslateLocalToWorld(Coord2df& worldCoordinates){
	if(mParent==NULL){return;}
	worldCoordinates=worldCoordinates + mPositionLocal;
	mParent->TranslateLocalToWorld(worldCoordinates);
}

void MapNode::TranslateWorldToLocal(Coord2df& worldCoordinates){
	if(mParent==NULL){return;}
	worldCoordinates=worldCoordinates - mPositionLocal;
	mParent->TranslateWorldToLocal(worldCoordinates);
}

bool MapNode::RemoveChild(MapNode* child){
	mChildren.erase(child);
}

void MapNode::AddChild	 (MapNode* child){
	mChildren.insert(child);
}


ComponentPosition::ComponentPosition(EID id, MapNode* parent, const std::string& logName) : BaseComponent(id, logName){
	mEntityID=id;
	mNode=new MapNode(parent);
	maximumSpeed = 15.0;
}
ComponentPosition::~ComponentPosition(){
	if(mNode!=NULL){
		delete mNode;
	}
}

void ComponentPosition::HandleEvent(const Event* event){

}

Coord2d ComponentPosition::GetPositionWorldInt(){
	return Coord2d( floor(mNode->mPositionWorld.x + 0.5f), floor(mNode->mPositionWorld.y + 0.5f) );
}
Coord2d ComponentPosition::GetPositionLocalInt(){
	return Coord2d( floor(mNode->mPositionLocal.x + 0.5f), floor(mNode->mPositionLocal.y + 0.5f) );
}

void ComponentPosition::Update(){
	if		(mMovement.x > maximumSpeed ){mMovement.x =  maximumSpeed;}
	else if (mMovement.x < -maximumSpeed){mMovement.x = -maximumSpeed;}
	if		(mMovement.y > maximumSpeed ){mMovement.y =  maximumSpeed;}
	else if (mMovement.y < -maximumSpeed){mMovement.y = -maximumSpeed;}

	mNode->mPositionLocal= mNode->mPositionLocal + mMovement;
	mMovement= mMovement + mAcceleration;
}

void ComponentPosition::ChangeParent(EID id){
	ComponentPosition* pos=((ComponentPosition*)((ComponentPositionManager*)mManager)->GetComponent(id));
	MapNode* node=NULL;
	if(pos==NULL){
		node=((ComponentPositionManager*)mManager)->GetRootNode();
	}
	else{node=pos->GetMapNode();}

	mNode->ChangeParent(node);
}

void ComponentPositionManager::AddComponent(EID id){
	AddComponent(id, mRootNode);
}

void ComponentPositionManager::AddComponent(EID id, MapNode* parent){
	compMapIt i=componentList.find(id);
	if(i!=componentList.end()){return;}
	ComponentPosition* pos=new ComponentPosition(id, parent, logFileName);
	pos->mManager=this;
	componentList[id]=pos;
}

void ComponentPosition::IncrementPosition(Coord2df pos){
	mNode->mPositionLocal=mNode->mPositionLocal + pos;
	mNode->mPositionWorld=mNode->mPositionWorld + pos;
}
void ComponentPosition::IncrementMovement(Coord2df mov){
	mMovement= mMovement+mov;
}
void ComponentPosition::IncrementAcceleration(Coord2df accel){
	mAcceleration=mAcceleration + accel;
}

void ComponentPosition::RoundPosition(){
	mNode->mPositionLocal.x=(floor(mNode->mPositionLocal.x + 0.5f));
	mNode->mPositionLocal.y=(floor(mNode->mPositionLocal.y + 0.5f));
}

//The problem with just setting the position, is that you can't also update the world position
//I think that's what's making characters sink into the terrain
void ComponentPosition::SetPositionLocal	(Coord2df pos){
	mNode->mPositionLocal	 = pos;
	MapNode* root= mNode->GetRootNode();
	root->UpdateWorld(root->mPositionLocal, true);
}
void ComponentPosition::SetPositionLocalInt    (Coord2d pos){
	mNode->mPositionLocal.x    = pos.x;
	mNode->mPositionLocal.y    = pos.y;
	MapNode* root= mNode->GetRootNode();
	root->UpdateWorld(root->mPositionLocal, true);
}
void ComponentPosition::SetMovement			(Coord2df mov){mMovement=mov;}
void ComponentPosition::SetAcceleration		(Coord2df acl){mAcceleration=acl;}

void ComponentPosition::SetPositionLocalX	(float x){
	mNode->mPositionLocal.x    =  x;
	MapNode* root= mNode->GetRootNode();
	root->UpdateWorld(root->mPositionLocal, true);
}
void ComponentPosition::SetMovementX		(float x){mMovement.x=x;}
void ComponentPosition::SetAccelerationX	(float x){mAcceleration.x=x;}

void ComponentPosition::SetPositionLocalY	(float y){
	mNode->mPositionLocal.y    =  y;
	MapNode* root= mNode->GetRootNode();
	root->UpdateWorld(root->mPositionLocal, true);
}
void ComponentPosition::SetMovementY		(float y){mMovement.y=y;}
void ComponentPosition::SetAccelerationY	(float y){mAcceleration.y=y;}

void ComponentPositionManager::Update(){
	compMapIt i=componentList.begin();
	for(; i!=componentList.end(); i++){
		i->second->Update();
	}

	mRootNode->UpdateWorld(mRootNode->mPositionLocal, false);
}

ComponentPositionManager::ComponentPositionManager(EventDispatcher* e) : BaseComponentManager("LOG_COMP_POSITION", e){
	mRootNode=new MapNode(NULL);
}

ComponentPositionManager::~ComponentPositionManager(){
	delete mRootNode;
}

Coord2df ComponentPosition::TranslateWorldToLocal(Coord2df world){
	mNode->mParent->TranslateWorldToLocal(world);
	return world;
}

Coord2df ComponentPosition::TranslateLocalToWorld(Coord2df local){
	mNode->mParent->TranslateLocalToWorld(local);
	return local;
}


