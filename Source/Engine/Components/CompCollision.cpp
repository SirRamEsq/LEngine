#include "CompCollision.h"
#include "../Kernel.h"
#include "math.h"


const TiledTileLayer* TColPacket::GetLayer(){
    return tl;
}

RSC_Heightmap TColPacket::GetHmap(){
    if(tl->UsesHMaps()==false){
        RSC_Heightmap l;
        return l;
    }
    GID id = tl->GetGID(x,y);
    TiledSet* ts = (TiledSet*)K_StateMan.GetCurrentState()->GetCurrentMap()->GetTiledData()->gid.GetItem(id);
    return ts->GetHeightMap(id);
}

bool ComponentCollision::ColBox::operator < (const ColBox &rhs) const {
    if( ((flags&PRIMARY)==PRIMARY)and((rhs.flags&PRIMARY)==PRIMARY) ){
            if(mOrderNum>=rhs.mOrderNum)  {return true;}
            else                            {return false;}
    }
    if((flags&PRIMARY)==PRIMARY){return true;}
    if((rhs.flags&PRIMARY)==PRIMARY){return false;}

    if( ((flags&ALWAYS_CHECK)==ALWAYS_CHECK)and((rhs.flags&ALWAYS_CHECK)==ALWAYS_CHECK) ){
            if(mOrderNum>=rhs.mOrderNum)  {return true;}
            else                            {return false;}
    }
    if((flags&ALWAYS_CHECK)==ALWAYS_CHECK){return true;}
    if((rhs.flags&ALWAYS_CHECK)==ALWAYS_CHECK){return false;}

    if( ((flags&ENT_CHECK)==ENT_CHECK)and((rhs.flags&ENT_CHECK)==ENT_CHECK) ){
            if(mOrderNum>=rhs.mOrderNum)  {return true;}
            else                            {return false;}
    }
    if((flags&ENT_CHECK)==ENT_CHECK){return true;}
    if((rhs.flags&ENT_CHECK)==ENT_CHECK){return false;}

    if(mOrderNum>=rhs.mOrderNum)  {return true;}
    else                            {return false;}

    return true;
}

ComponentCollision::ComponentCollision(EID id, ComponentPosition* pos, ComponentCollisionManager* manager)
: BaseComponent(id, manager), myPos(pos){
    alwaysCheckCount=0;
}

ComponentCollision::~ComponentCollision(){
    myPos=NULL;
}

void ComponentCollision::Update(){}

void ComponentCollisionManager::SetDependencies(ComponentPositionManager* pos){
	dependencyPosition = pos;
}

ComponentCollision::ColBox::ColBox(Rect r, int i, ComponentPosition* pos, uint8_t f, int orderNum, MAP_DEPTH d) : myPos(pos){
    if(r.w==0){r.w=1;}
    if(r.h==0){r.h=1;}
    if(myPos == NULL){
        LOG_INFO("ComponentCollision::ColBox::Colbox::myPos is NULL");
    }
    rect=r;
    id=i;
    flags=f;
    mOrderNum=orderNum;
    depth=d;
    active=true;
}

void ComponentCollision::AddCollisionBoxInt(int x, int y, int w, int h, int boxid, int ordernNum){
    Rect rect(x,y,w,h);
    AddCollisionBox(rect, boxid, ordernNum);
}

void ComponentCollision::AddCollisionBox(Rect rect, int boxid, int orderNum){
    ColBox cb(rect, boxid, myPos, 0, orderNum);
    cb.myPos=myPos;
    boxes.push_back(cb);
    OrderList();
}

void ComponentCollision::SetPrimaryCollisionBox(int boxid){
    ColBox* cb=GetColBox(boxid);
    if(cb!=NULL){cb->flags= (cb->flags|PRIMARY);}
    else{
        std::stringstream ss;
        ss << "Couldn't find box id: " <<  boxid;
        LOG_INFO(ss.str());
        return;
    }
    OrderList();
}

void ComponentCollision::AlwaysCheck(int boxid){
    ColBox* cb=GetColBox(boxid);
    if(cb!=NULL){cb->flags= (cb->flags | ALWAYS_CHECK);}
    alwaysCheckCount+=1;
    OrderList();
}

void ComponentCollision::CheckForEntities(int boxid){
    ColBox* cb=GetColBox(boxid);
    if(cb!=NULL){cb->flags= (cb->flags | ENT_CHECK);}
    OrderList();
}

void ComponentCollision::CheckForTiles(int boxid){
    ColBox* cb=GetColBox(boxid);
    if(cb!=NULL){cb->flags= (cb->flags | TILE_CHECK);}
    OrderList();
}

void ComponentCollision::Activate(int boxid){
    ColBox* cb=GetColBox(boxid);
    cb->active=true;
}
void ComponentCollision::Deactivate(int boxid){
    ColBox* cb=GetColBox(boxid);
    cb->active=false;
}

ComponentCollision::ColBox* ComponentCollision::GetColBox(int boxid){
    for(auto i=boxes.begin(); i!=boxes.end(); i++){
        if((*i).id==boxid){return &(*i);}
    }
    return NULL;
}

Rect* ComponentCollision::GetPrimaryRect(){
    auto i=boxes.begin();
    if(i==boxes.end()){return NULL;}

    //if the first box isn't primary, none of them are
    if( (i->flags&PRIMARY)==PRIMARY ){return &i->rect;}
    return NULL;
}

ComponentCollision::ColBox* ComponentCollision::GetPrimary(){
    auto i=boxes.begin();
    if(i==boxes.end()){return NULL;}

    //if the first box isn't primary, none of them are
    if( (i->flags&PRIMARY) ){return &(*i);}
    return NULL;
}

Rect* ComponentCollision::GetBox(int boxid){
    ColBox* cb=GetColBox(boxid);
    if(cb==NULL){return NULL;}
    return &cb->rect;
}

void ComponentCollision::OrderList(){
    //primary comes first,
    //then always checks,
    //then entities,
    //then tiles
    std::sort(boxes.begin(), boxes.end());
}

const Rect& ComponentCollision::ColBox::ToGameCoords(){
	auto pos= myPos->GetPositionWorld().Round();

    gameCoords.x = pos.x + rect.x;
    gameCoords.y = pos.y + rect.y;
    gameCoords.w = rect.w;
    gameCoords.h = rect.h;
    return gameCoords;
}

void ComponentCollision::ChangeWidth(int boxid, int value){
    Rect* cb;
    if((cb=GetBox(boxid))!=NULL){
        if(value==0){value=1;}
        cb->w=value;
    }
}
void ComponentCollision::ChangeHeight(int boxid, int value){
    Rect* cb;
    if((cb=GetBox(boxid))!=NULL){
        if(value==0){value=1;}
        cb->h=value;
    }
}
void ComponentCollision::ChangeX(int boxid, int value){
    Rect* cb;
    if((cb=GetBox(boxid))!=NULL){
        cb->x=value;
    }
}
void ComponentCollision::ChangeY(int boxid, int value){
    Rect* cb;
    if((cb=GetBox(boxid))!=NULL){
        cb->y=value;
    }
}
void ComponentCollision::ChangeBox(int boxid, Rect& box){
    Rect* cb;
    if((cb=GetBox(boxid))!=NULL){
        cb->x=box.x;
        cb->y=box.y;
        cb->w=box.w;
        cb->h=box.h;
    }
}

///////////////////////////////
//Component Collision Manager//
///////////////////////////////

ComponentCollisionManager::ComponentCollisionManager(EventDispatcher* e) : BaseComponentManager_Impl(e){

}

std::unique_ptr<ComponentCollision>ComponentCollisionManager::ConstructComponent(EID id, ComponentCollision* parent){
    auto comp = std::make_unique<ComponentCollision>(id, dependencyPosition->GetComponent(id), this);
	return std::move(comp);
}

void ComponentCollisionManager::SendCollisionEvent(const ComponentCollision& sender, const ComponentCollision& reciever, int recieverBoxID, Event::MSG mes){
    EColPacket ePacket;
    ePacket.name      = sender.name;
    ePacket.objType   = sender.objType;
    ePacket.box       = recieverBoxID;

	EColPacket::ExtraDataDefinition extraData(&ePacket);
	Event event(sender.GetEID(), reciever.GetEID(), mes, "ENTITY_COLLISION", &extraData);
    eventDispatcher->DispatchEvent(event);
}

void ComponentCollisionManager::UpdateCheckEntityCollision(){
    ComponentCollision* comp1 = NULL;
    ComponentCollision* comp2 = NULL;

    ComponentCollision::ColBox* primaryBox1;
    ComponentCollision::ColBox* primaryBox2;

    //These sets ensure only one collision is sent per box
    //if the other entity has multiple colboxes that collide with the first entity's only collision box
    //these sets ensure that the first entity will only recieve one event for it's only collision box
    //whereas the other will get a collision event for each of its boxes
    std::set <int> alreadyRegisteredBox1;
    std::set <int> alreadyRegisteredBox2;

    Rect box1(0,0,0,0);
    Rect box2(0,0,0,0);

    bool primaryPass;

	/* ISSUE
	 * collision boxes can be part of multiple buckets
	 * therefore, two eids can be compared twice
	 */

	std::set< std::pair<EID, EID> > alreadyProcessed;
    for(auto bucketIt = grid.buckets.begin(); bucketIt != grid.buckets.end(); bucketIt++){
        for(auto eidIt1 = bucketIt->second.begin(); eidIt1 != bucketIt->second.end(); eidIt1++){

            comp1 = componentList[*eidIt1].get();

            for(auto eidIt2 = eidIt1+1; eidIt2 != bucketIt->second.end(); eidIt2++){
				auto newPair = std::pair<EID,EID>(*eidIt1, *eidIt2);
				if(alreadyProcessed.find(newPair) != alreadyProcessed.end()){continue;}
				alreadyProcessed.insert(newPair);

                comp2 = componentList[*eidIt2].get();
                primaryPass=false;

                alreadyRegisteredBox1.clear();
                alreadyRegisteredBox2.clear();

                primaryBox1 = comp1->GetPrimary();
                primaryBox2 = comp2->GetPrimary();

                //"ALWAYS_CHECK" is ignored since the advent of buckets

                //Check first if the primaries collide
                if( (primaryBox1 != NULL) && (primaryBox2 != NULL) ){
                    primaryPass=CollisionRectRect(primaryBox1->ToGameCoords(), primaryBox2->ToGameCoords()).mCollided;
                }
                if(primaryPass){
                    for(auto boxIt1=comp1->GetItBeg(); boxIt1!=comp1->GetItEnd(); boxIt1++){//iterate through the collision boxes of compIt1
                        if( (boxIt1->flags&ENT_CHECK)!=ENT_CHECK)            {continue;} //Skip if you the box doesn't have the ENT_CHECK flag set
                        if(not boxIt1->active){continue;}
                        box1=boxIt1->ToGameCoords();

                        for(auto boxIt2=comp2->GetItBeg(); boxIt2!=comp2->GetItEnd(); boxIt2++){//iterate through the collision boxes of compIt2
                            if((boxIt2->flags&ENT_CHECK)!=ENT_CHECK)            {continue;}
                            if(not boxIt2->active){continue;}
                            box2=boxIt2->ToGameCoords();

                            if(CollisionRectRect(box1, box2).mCollided){
                                //Each entity will only be sent one collision event per collision box max
                                if(alreadyRegisteredBox1.find(boxIt1->id)==alreadyRegisteredBox1.end()){
                                    //                      Sender                                      Reciever                       Reciever BoxID
                                    SendCollisionEvent(*comp2, *comp1 ,boxIt1->id, Event::MSG::COLLISION_ENTITY);
                                    //Register box to make sure an event isn't sent here again
                                    alreadyRegisteredBox1.insert(boxIt1->id);
                                }

                                if(alreadyRegisteredBox2.find(boxIt2->id)==alreadyRegisteredBox2.end()){
                                    //                      Sender                                      Reciever                       Reciever BoxID
                                    SendCollisionEvent(*comp1, *comp2 ,boxIt2->id, Event::MSG::COLLISION_ENTITY);
                                    //Register box to make sure an event isn't sent here again
                                    alreadyRegisteredBox2.insert(boxIt2->id);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void ComponentCollisionManager::UpdateCheckTileCollision(const RSC_Map* currentMap){
    //Put event into smart pointer so that the same event can be reused (not multiple events allocated and deallocated on the stack)
    //May want to change this behaviour at some point, as recievers of the event may expect that they can hold on to it
    Coord2df ul(0,0), dr(0,0);
    const TiledTileLayer* tLayer=NULL;
    Rect box1(0,0,0,0);
    Rect box2(0,0,0,0);
    TColPacket packet;//for messaging purposes

    for(auto compIt1=componentList.begin(); compIt1!=componentList.end(); compIt1++){
        if(currentMap==NULL){continue;}//No point in checking if there's no map to check against

        //Start iterating through collision boxes
        //boxIt1 is a colbox iterator for each collision component
        //i iterates through all of the collision components
        for(auto boxIt1=compIt1->second.get()->GetItBeg(); boxIt1!=compIt1->second.get()->GetItEnd(); boxIt1++){
            if((boxIt1->flags&TILE_CHECK)!=TILE_CHECK)   {continue;} //Check if the box even exists to check tiles

            //Adds box coordinates to entity's coordinates
            box1 = boxIt1->ToGameCoords();

            ul.x = box1.GetLeft();  ul.y = box1.GetTop();
            dr.x = box1.GetRight(); dr.y = box1.GetBottom();
            int txx1=ul.x;
            int txx2=dr.x;
            int tyy1=ul.y;
            int tyy2=dr.y;

            CoordToGrid(txx1,tyy1);
            CoordToGrid(txx2,tyy2);
            if( (txx1==txx2) and (tyy1==tyy2) ){//if the top left is the same as the bottom right, then the whole box has fit inside a single tile
                tLayer = currentMap->GetTileLayerCollision(txx1, tyy1, true);

                if  (tLayer==NULL) {continue;}

                packet.tl=tLayer;
                packet.x=txx1;
                packet.y=tyy2;
                packet.box=boxIt1->id;

				TColPacket::ExtraDataDefinition extraData(&packet);
				Event event(EID_SYSTEM, compIt1->first, Event::MSG::COLLISION_TILE, "TILE", &extraData);
                eventDispatcher->DispatchEvent(event);

                continue;
            }

            int differenceX=txx2-txx1; //Both differences will always be positive
            int differenceY=tyy2-tyy1;

            bool negativeH=(box1.h<0);
            bool negativeW=(box1.w<0);
            int tx, ty;

            if(negativeW)   {tx=txx2;}
            else            {tx=txx1;}
            if(negativeH)   {ty=tyy2;}
            else            {ty=tyy1;}

            bool breakOut=false;
            for(int iter=0; iter<=differenceX; iter++){
                for(int iter2=0; iter2<=differenceY; iter2++){
                    tLayer=currentMap->GetTileLayerCollision(tx, ty, true);
                    if(tLayer!=NULL){
                        packet.x=tx;
                        packet.y=ty;
                        packet.box=boxIt1->id;
                        packet.tl=tLayer;

						TColPacket::ExtraDataDefinition extraData(&packet);
						Event event(EID_SYSTEM, compIt1->first, Event::MSG::COLLISION_TILE, "TILE", &extraData);
						eventDispatcher->DispatchEvent(event);

                        breakOut=true;
                    }
                    if(!negativeH)  {ty+=1;}
                    else            {ty-=1;}
                    if(breakOut)    {break;}
                }
                if(negativeH)   {ty=tyy2;}
                else            {ty=tyy1;}
                if(!negativeW)  {tx+=1;}
                else            {tx-=1;}
                if(breakOut)    {break;}
            }
        }
    }
}


void ComponentCollisionManager::UpdateBuckets (int widthPixels){
    grid.UpdateBuckets(&componentList, widthPixels);
}

void CollisionGrid::UpdateBuckets(const std::unordered_map<EID, std::unique_ptr<ComponentCollision> >* comps, int mapWidthPixels){
    std::set<int> hashes;
    ComponentCollision::ColBox* primaryBox;
    Rect rect;

	//Collision will get WEIRD once outside the map
	
    buckets.clear();
    for (auto it = comps->cbegin(); it != comps->cend(); it++){
        primaryBox = it->second->GetPrimary();
        if(primaryBox == NULL) {continue;}

        rect = primaryBox->ToGameCoords();

        hashes.insert( (rect.GetLeft()  / COLLISION_GRID_SIZE) + ( (rect.GetTop()    / COLLISION_GRID_SIZE) * mapWidthPixels) );
        hashes.insert( (rect.GetRight() / COLLISION_GRID_SIZE) + ( (rect.GetTop()    / COLLISION_GRID_SIZE) * mapWidthPixels) );
        hashes.insert( (rect.GetLeft()  / COLLISION_GRID_SIZE) + ( (rect.GetBottom() / COLLISION_GRID_SIZE) * mapWidthPixels) );
        hashes.insert( (rect.GetRight() / COLLISION_GRID_SIZE) + ( (rect.GetBottom() / COLLISION_GRID_SIZE) * mapWidthPixels) );

        for(auto hashIt = hashes.begin(); hashIt != hashes.end(); hashIt++){
            buckets[*hashIt].push_back(it->second->GetEID());
        }
        hashes.clear();
    }
}

void ComponentCollisionManager::Update(){
    /*
    SUMMARY:

    Function will check for collisions against entities and tiles and send the results to the script component via events.

    COMPONENTCHECKING:

    Function will first check whether two collision components have primary collision boxes. If so,
    the primary boxes are checked and the value primaryPass is the result (it is false by default).
    The function then begins iterating through all of the boxes of each component.
    The box is skipped over under the following conditions:
    -The box is primary (unimportant as we already checked the primary boxes)
    -The box is meant for tiles (tiles will be checked against each object later)
    -The box isn't meant to always be checked and the primaryPass is false

    The components then each recieve an event if the boxes collide. The component's eid
    that was collided with is the reciever and the boxid is the extra data.
    */

	/// \TODO make this manager respect the parent child relationship
	GameStateManager* gs = &K_StateMan;
	GameState* state = gs->GetCurrentState();
	auto stateMap = state->GetCurrentMap();
	if(stateMap == NULL){return;}

    //update grid
    UpdateBuckets(stateMap->GetWidthPixels());
    UpdateCheckEntityCollision();
    UpdateCheckTileCollision(K_StateMan.GetCurrentState()->GetCurrentMap());
}

//////////////
//EColPcaket//
//////////////
EColPacket::ExtraDataDefinition::ExtraDataDefinition(const EColPacket* p)
	: packet(p){
}

void EColPacket::ExtraDataDefinition::SetExtraData(Event* event){
	event->extradata = packet;
}

const EColPacket* EColPacket::ExtraDataDefinition::GetExtraData(const Event* event){
	return static_cast<const EColPacket*>(event->extradata);
}

//////////////
//TColPcaket//
//////////////
TColPacket::ExtraDataDefinition::ExtraDataDefinition(const TColPacket* p)
	: packet(p){
}

void TColPacket::ExtraDataDefinition::SetExtraData(Event* event){
	event->extradata = packet;
}

const TColPacket* TColPacket::ExtraDataDefinition::GetExtraData(const Event* event){
	return static_cast<const TColPacket*>(event->extradata);
}
