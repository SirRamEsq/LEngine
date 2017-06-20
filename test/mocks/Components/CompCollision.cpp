#include "CompCollision.h"
#include "../Kernel.h"
#include "math.h"


const TiledTileLayer* TColPacket::GetLayer(){
    return tl;
}

LHeightmap TColPacket::GetHmap(){
    /*if(tl->UsesHMaps()==false){
        LHeightmap l;
        return l;
    }
    GID id = tl->GetGID(x,y);
    TiledSet* ts = (TiledSet*)K_StateMan.GetCurrentState()->GetCurrentMap()->tiledData.get()->gid.GetItem(id);
    return null;*/
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

ComponentCollision::ComponentCollision(EID id, Coord2df pos, const std::string& logName)
: BaseComponent(id, logName), myPos(pos){
    noTiles=false;
    alwaysCheckCount=0;
}

ComponentCollision::~ComponentCollision(){

}

void ComponentCollision::Update(){}
void ComponentCollision::HandleEvent(const Event* event){}

ComponentCollision::ColBox::ColBox(CRect r, int i, Coord2df pos, uint8_t f, int orderNum, MAP_DEPTH d) : myPos(pos){
    if(r.w==0){r.w=1;}
    if(r.h==0){r.h=1;}
    rect=r;
    id=i;
    flags=f;
    mOrderNum=orderNum;
    depth=d;
    active=true;
}

void ComponentCollision::AddCollisionBoxInt(int x, int y, int w, int h, int boxid, int ordernNum){
    CRect rect(x,y,w,h);
    AddCollisionBox(rect, boxid, ordernNum);
}

void ComponentCollision::AddCollisionBox(CRect rect, int boxid, int orderNum){
    ColBox cb(rect, boxid, myPos, 0, orderNum);
    cb.myPos=myPos;
    boxes.push_back(cb);
    OrderList();
}

void ComponentCollision::SetPrimaryCollisionBox(int boxid, bool ntile){
    noTiles=ntile;
    ColBox* cb=GetColBox(boxid);
    if(cb!=NULL){cb->flags= (cb->flags|PRIMARY);}
    else{
        std::stringstream ss;
        ss << "Couldn't find box id: " <<  boxid;
        ErrorLog::WriteToFile(ss.str(), logFileName);
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

CRect* ComponentCollision::GetPrimaryRect(){
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

CRect* ComponentCollision::GetBox(int boxid){
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

const CRect& ComponentCollision::ColBox::ToGameCoords(){
    Coord2d pos;
    pos.x = myPos.x;
    pos.y = myPos.y;
    gameCoords.x = pos.x + rect.x;
    gameCoords.y = pos.y + rect.y;
    gameCoords.w = rect.w;
    gameCoords.h = rect.h;
    return gameCoords;
}

void ComponentCollision::ChangeWidth(int boxid, int value){
    CRect* cb;
    if((cb=GetBox(boxid))!=NULL){
        if(value==0){value=1;}
        cb->w=value;
    }
}
void ComponentCollision::ChangeHeight(int boxid, int value){
    CRect* cb;
    if((cb=GetBox(boxid))!=NULL){
        if(value==0){value=1;}
        cb->h=value;
    }
}
void ComponentCollision::ChangeX(int boxid, int value){
    CRect* cb;
    if((cb=GetBox(boxid))!=NULL){
        cb->x=value;
    }
}
void ComponentCollision::ChangeY(int boxid, int value){
    CRect* cb;
    if((cb=GetBox(boxid))!=NULL){
        cb->y=value;
    }
}
void ComponentCollision::ChangeBox(int boxid, CRect& box){
    CRect* cb;
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

ComponentCollisionManager::ComponentCollisionManager() : BaseComponentManager("L_COMP_COLLISION"){

}

void ComponentCollisionManager::AddComponent(EID id){
    compMapIt i=componentList.find(id);
    if(i!=componentList.end()){return;}
    ComponentCollision* cbox=new ComponentCollision(id, Coord2df(10, 10), logFileName);
    componentList[id]=cbox;
}

void ComponentCollisionManager::SendCollisionEvent(const ComponentCollision& sender, const ComponentCollision& reciever, int recieverBoxID, MESSAGE_TYPE mes){
    EColPacket ePacket;
    ePacket.name      = sender.name;
    ePacket.objType   = sender.objType;
    ePacket.box       = recieverBoxID;

    K_EventMan.DispatchEvent(std::unique_ptr<Event>(new Event(sender.GetEID(), reciever.GetEID(), mes, &ePacket)));
}

void ComponentCollisionManager::UpdateCheckEntityCollision(){

}

void ComponentCollisionManager::UpdateCheckTileCollision(const LMap* currentMap){

}

void CollisionGrid::UpdateBuckets(std::map<EID, ComponentCollision*>* comps, const LMap* currentMap){

}

void ComponentCollisionManager::Update(){

}
