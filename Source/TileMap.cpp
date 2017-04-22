#include "TileMap.h"
#include "Kernel.h"

MapBackground::MapBackground(std::string bg, MAP_DEPTH depth){
    sprite=NULL;
    background=NULL;

    bgstr=bg;
    ReloadBG();

    sprstr=background->spriteName;
    anistr=background->animationName;
    ReloadSprite();

    SetDepth(depth);
    oldCamX=0;
    oldCamY=0;
    offsetX=0;
    offsetY=0;
}

MapBackground::~MapBackground(){
    if(sprite!=NULL){
        delete sprite;
    }
    if(background!=NULL){
        delete background;
    }
}

void MapBackground::ReloadSprite(){
    if(sprite!=NULL){delete sprite;}

    LSprite* tspr=K_SpriteMan->GetItem(sprstr);
    if(tspr!=NULL){
        sprite=new LSprite("NULL");
        *sprite=*tspr;
        sprite->SetCurrentAnimation(anistr);
    }
    else{sprite=NULL;}
}

void MapBackground::ReloadBG(){
    if(background!=NULL){delete background;}

    LBackground* tbg=K_BGMan->GetItem(bgstr);
    if(tbg!=NULL){
        background=new LBackground();
        *background=*tbg;
    }
    else{background=NULL;}
}

void MapBackground::Render(){
    if(sprite==NULL){return;}
    bool rptX=(background->flags&BG_REPEATX);
    bool rptY=(background->flags&BG_REPEATY);
    int width=background->width;
    int height=background->height;
    int x= background->x;
    int y= background->y;

    CRect cam=K_RenderMan->camera.GetRect();
    int minValueW=cam.x-width;
    int minValueH=cam.y-height;
    int maxValueW=cam.GetRight()+width;
    int maxValueH=cam.GetBottom()+height;

    offsetX+=(background->paralaxSpeed*(cam.x-oldCamX));
    offsetY+=(background->paralaxSpeed*(cam.y-oldCamY));
    while(offsetX>width){offsetX-=width;}
    while(offsetX<-width){offsetX+=width;}
    while(offsetY>height){offsetY-=height;}
    while(offsetY<-height){offsetY+=height;}

    if(!rptX){minValueW=0;maxValueW=1;}
    if(!rptY){minValueH=0;maxValueH=1;}

    for(int i=minValueW; (i+x)<maxValueW; i+=width){
        for(int ii=minValueH; (ii+y)<maxValueH; ii+=height){
            sprite->DrawSprite(i+x-offsetX, ii+y-offsetY, 1, 1, 0, L_COLOR_WHITE);
        }
    }
    oldCamX=cam.x;
    oldCamY=cam.y;
}

MapEvent::MapEvent(std::string eve, int xx, int yy, int ww, int hh, MAP_DEPTH d){
    eventName=eve;
    coords.x=xx;
    coords.y=yy;
    coords.w=ww;
    coords.h=hh;
}
MapEvent::MapEvent(std::string eve, CRect coord, MAP_DEPTH d){
    eventName=eve;
    coords=coord;
}

void TileMap::DeleteEvent(EventListIt i){
    delete *i;
    events.erase(i);
}

bool TileMap::AddEvent(MapEvent* event){
    events.push_back(event);
    return true;
}

bool TileMap::AddBackground(MapBackground* bg){
    BackgroundMap::iterator i=BGs.find(bg->GetDepth());
    if(i!=BGs.end()){return false;}
    BGs[bg->GetDepth()]=bg;
    return true;
}

MapBackground* TileMap::GetBackground(MAP_DEPTH d){
    BackgroundMap::iterator i=BGs.find(d);
    if(i!=BGs.end()){return (i->second);}
    return NULL;
}

void TileMap::DeleteBackground(MAP_DEPTH d){
    BackgroundMap::iterator i=BGs.find(d);
    if(i==BGs.end()){return;}
    delete i->second;
    BGs.erase(d);
}

void TileMap::Startup(unsigned int w, unsigned int h){
    AddTileDef(BLANK_TILE_NAME, BLANK_SPRITE_NAME, BLANK_ANIMATION_NAME, BLANK_HMAP_NAME, 0, 0); //Add dummy TDF
    if(GetTileDef(BLANK_TILE_NAME)==NULL){ //Add and check for dummy tiledef
        ErrorLog::WriteToFile("Cannot create");
        ErrorLog::WriteToFile(BLANK_TILE_NAME);
        ErrorLog::WriteToFile("Tiledef");
    }
    width=w;
    height=h;

    //Layer 0 is always initalized
    InitTileLayer(0);
}

TileMap::~TileMap(){
    ClearMaps();
    ClearDefs();
    ClearBackgrounds();
    ClearEvents();
}

void TileMap::ClearMaps(){
    tileMap.clear();
    registeredDepths.clear();
    std::list<RenderTileLayer*>::iterator i=renderPointers.begin();
    for(; i!=renderPointers.end(); i++){
        delete *i;
    }
    renderPointers.clear();
}

void TileMap::ClearDefs(){
    std::map<std::string, LTDF*>::iterator i=defs.begin();
    for(; i!=defs.end(); i++){
        delete i->second;
    }
    defs.clear();
}

void TileMap::ClearBackgrounds(){
    BackgroundMap::iterator i=BGs.begin();
    for(; i!=BGs.end(); i++){
        delete i->second;
    }
    BGs.clear();
}

void TileMap::ClearEvents(){
    EventListIt i=events.begin();
    for(; i!=events.end(); i++){
        delete *i;
    }
    events.clear();
}

void TileMap::InitTileLayer(MAP_DEPTH depth){
    //Check if this depth has already been initalized
    TMAP3D::iterator i;
    i=tileMap.find(depth);
    if(i!=tileMap.end()){return;}

    if(GetTileDef(BLANK_TILE_NAME)==NULL){
        ErrorLog::WriteToFile("Cannot access");
        ErrorLog::WriteToFile(BLANK_TILE_NAME);
        ErrorLog::WriteToFile("Tiledef");
    }

    //initalize new 2d tmap
    TMAP2D tiles( width, std::vector<LTDF*> ( height ) );

    //set each tile to the dummy value
    for(unsigned int i=0; i<width; i++){
        for(unsigned int ii=0; ii<height; ii++){
            tiles[i][ii]=GetTileDef(BLANK_TILE_NAME);
        }
    }

    tileMap[depth]=tiles;
    renderPointers.push_back( new RenderTileLayer(&tileMap[depth], depth) );
    registeredDepths.push_back(depth);
}

bool TileMap::AddTileDef(std::string name, std::string spr, std::string ani, std::string hmap, double friction, int32_t flags){
    DefMap::iterator i;
    i=defs.find(name);

    if(i!=defs.end()){
        return false;
    }

    else if((name==BLANK_TILE_NAME)and(spr==BLANK_SPRITE_NAME)and(ani==BLANK_ANIMATION_NAME)and (hmap==BLANK_HMAP_NAME) ){ //If the names match the dummy tile's names
        defs[name]=new LTDF(name, spr, ani, hmap, friction, flags);
        return true;
    }

    defs[name]=new LTDF(name, spr, ani, hmap, friction, flags);
    defs[name]->LoadSprite();
    return true;
}

bool TileMap::AddTileDef(std::string name){
    DefMap::iterator i;
    i=defs.find(name);

    if(i!=defs.end()){
        return false;
    }

    if(K_TDFMan->GetItem(name)==NULL){
        ErrorLog::WriteToFile(name, " TDF doesnt exist");
        //return false;
    }

    LTDF* tdf= new LTDF("NULL", "NULL", "NULL", "NULL", 0, 0);
    *tdf=*(K_TDFMan->GetItem(name));

    tdf->LoadSprite();
    defs[name]=tdf;
    return true;
}

LTDF* TileMap::GetTileDef(std::string name){
    DefMap::iterator i;
    i=defs.find(name);

    if(i!=defs.end()){return defs[name];}

    else{return NULL;}
}

void TileMap::DeleteTileDef(std::string name){
    DefMap::iterator i;
    i=defs.find(name);

    if(i==defs.end()){ErrorLog::WriteToFile("DeleteTileDef was pased an invalid TileDef name"); return;}

    DEPTHLIST::iterator it=registeredDepths.begin();
    for(; it!=registeredDepths.end(); it++){
        for(unsigned int xi=0; xi<width; xi++){
            for(unsigned int yi=0; yi<height; yi++){
                if( tileMap[(*it)][xi][yi]==(*i).second ){tileMap[(*it)][xi][yi]=GetTileDef(BLANK_TILE_NAME);}
            }
        }
    }
    LTDF* def=(*i).second;
    defs.erase(name);
    delete def;
}

bool TileMap::SetTile(std::string name, unsigned int x, unsigned int y, MAP_DEPTH depth){
    if((x>=width) or (y>=height)){
        ErrorLog::WriteToFile("Error! Out of bounds coordinates in SetTile Function");
        ErrorLog::WriteToFile("   ...X value is", x);
        ErrorLog::WriteToFile("   ...Width is  ", width);
        ErrorLog::WriteToFile("   ...Y value is", x);
        ErrorLog::WriteToFile("   ...Height is ", height);
        return false;
    }

    LTDF* tdf=GetTileDef(name);
    if(tdf==NULL){
        ErrorLog::WriteToFile("Error! Bad Tile Name passed in SetTile Function");
        ErrorLog::WriteToFile("   ... Name is ", name);
        return false;
    }

    TMAP3D::iterator i;
    i=tileMap.find(depth);
    if(i==tileMap.end()){
        ErrorLog::WriteToFile("Error! Bad Depth passed in SetTile Function");
        return false;
    }

    (*i).second[x][y]=tdf;
    return true;
}

void TileMap::CreateFromTiledMap(LMap* lm){
    TiledData* tiled=&(lm->tDat);
    Startup(tiled->width, tiled->height);
    std::list<TiledSet*>::iterator i;
    i=tiled->tileSets.begin();

    for(; i!=tiled->tileSets.end(); i++){

    }
}
