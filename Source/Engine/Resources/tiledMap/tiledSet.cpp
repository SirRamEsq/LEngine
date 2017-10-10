#include "tiledSet.h"
#include "../../Kernel.h"

TiledSet::TiledSet(const std::string& n, const std::string& tex, const unsigned int tileW, const unsigned int tileH, GID first, GIDManager* man)
    : GIDEnabled(man), name(n), textureName(tex), tileWidth(tileW), tileHeight(tileH){
    Init(first, man);
}
TiledSet::TiledSet(const TiledSet& rhs, GIDManager* g)
    : GIDEnabled(g), name(rhs.name), textureName(rhs.textureName), tileWidth(rhs.tileWidth), tileHeight(rhs.tileHeight){
    Init(rhs.GetFirstGID(), g);
    transparentColor = rhs.transparentColor;
    tileAnimations = rhs.tileAnimations;
    //tileHMAPs = rhs.tileHMAPs;
}

void TiledSet::Init(GID first, GIDManager* man){
    try{
        LoadTexture();
    }
    catch(LEngineFileException fe){
        std::stringstream ss;
        ss << fe.what() << "\n Filename is " << fe.fileName;
        LOG_INFO(ss.str());
        initializationOK=false;
        return;
    }
    if(texture==NULL){LOG_INFO("[C++; TiledSet Constructor] Couldn't load texture from name " + textureName);}

    tilesWide  = texture->GetWidth()  / tileWidth;
    tilesHigh  = texture->GetHeight() / tileHeight;
    tilesTotal = tilesWide * tilesHigh;

    SetFirstGID(first                 );
    SetLastGID (first + tilesTotal - 1); //Need to subtract one, as the firstGID counts as an index

    for(GID id=GetFirstGID(); id<=GetLastGID(); id++){
        LoadHeightMaps(id);
    }

    man->NewRange(GetFirstGID(), GetLastGID(), this);
    initializationOK=true;
}

bool TiledSet::ContainsTile(GID id) const {
    if( (id>=GetFirstGID()) and (id<=(GetFirstGID()+tilesTotal)) ){ return true; }
    return false;
}

void TiledSet::LoadTexture(){
    texture= K_TextureMan.GetItem(textureName);
    if(texture==NULL){
        K_TextureMan.LoadItem(textureName, textureName);
        texture=K_TextureMan.GetItem(textureName);
        if(texture==NULL){
            throw LEngineFileException("Couldn't load texture" + textureName, Log::typeDefault);
        }
    }
}

void TiledSet::GetTextureCoordinatesFromGID(GID id, float& left, float& right, float& top, float& bottom) const {
    if((ContainsTile(id)==false)or (initializationOK==false)){left=right=top=bottom=0.0f; return;}
    GID indexValue = id - GetFirstGID();
    unsigned int x = 0;
    unsigned int y = 0;

    while(indexValue>=tilesWide){
        y+=1;
        indexValue-=tilesWide;
    }
    x=indexValue;

    //Get Top Left
    float texX = (float)(x*16) / (float)(texture->GetWidth());
    float texY = (float)(y*16) / (float)(texture->GetHeight());
    left = texX;
    top  = texY;

    //Get Bottom Right
    texX    = (float)((x*16)+16) / (float)(texture->GetWidth());
    texY    = (float)((y*16)+16) / (float)(texture->GetHeight());
    right   = texX;
    bottom  = texY;
}

CRect TiledSet::GetTextureRectFromGID(GID id) const {
    if(ContainsTile(id)==false){return CRect(0,0,0,0);}
    GID indexValue = id - GetFirstGID();
    unsigned int x = 0;
    unsigned int y = 0;

    while(indexValue>=tilesWide){
        y+=1;
        indexValue-=tilesWide;
    }
    x=indexValue;
    CRect returnVal(x*16, y*16, 16, 16);
    return returnVal; //posX, posY, TileWidth, TileHeight
}

const LAnimation* TiledSet::GetAnimationDataFromGID (GID id) const{
    auto it = tileAnimations.find(id);
    if(it == tileAnimations.end()){return NULL;}
    return it->second;
}

void TiledSet::LoadHeightMaps(GID id){
    if(texture==NULL){return;}

    CRect coord=GetTextureRectFromGID(id);

    int8_t hHMap [16] = {0};
    int8_t vHMap [16] = {0};

    if((coord.w!=16)or(coord.h!=16)){
        RSC_Heightmap hmap(hHMap,vHMap);
        tileHMAPs[id]=hmap;
            return;
    }

    int xit=coord.x;
    int yit=coord.y;

    //First, figure out the origin of the image by comparing how many non alpha pixels there are between opposite sides
    //ie. if there are more solid pixels on the right than the left, the origin is at the right

    int topsolid=0;
    int bottomsolid=0;
    int leftsolid=0;
    int rightsolid=0;

    int8_t hmapValue=0;
    bool solid=false;
    bool topHit=false;
    bool bottomHit=false;
    bool leftHit=false;
    bool rightHit=false;
    //Figure out what pixels are visible along each border
    for (; xit<coord.GetRight(); xit++){
        //std::stringstream ss;
        //ss << "Right: " << coord.GetRight() <<  "Left: " << coord.GetLeft() <<  "Top: " << coord.GetTop() <<  "Bottom: " << coord.GetBottom();
        //LOG_INFO(ss.str());
        if(texture->GetPixelAlpha(xit,coord.GetTop())==255){ //top of image
           topsolid+=1;
           topHit=true;
        }
        if(texture->GetPixelAlpha(xit,coord.GetBottom()-1)==255){ //bottom of image
           bottomsolid+=1;
           bottomHit=true;
        }
        if((topHit)and(bottomHit)){
           hHMap[xit-coord.x]=16;
           //the height map was initialized to 0, this sets up the height map if top and bottom solid are equal
        }
        topHit=false;
        bottomHit=false;
    }
    for (; yit<coord.GetBottom(); yit++){
        if(texture->GetPixelAlpha(coord.GetLeft(), yit)==255){ //left of image
           leftsolid+=1;
           leftHit=true;
        }
        if(texture->GetPixelAlpha(coord.GetRight()-1, yit)==255){ //right of image
           rightsolid+=1;
           rightHit=true;
        }
        if((leftHit)and(rightHit)){
           vHMap[yit-coord.y]=16;
           //the height map was initialized to 0, this sets up the height map if top and bottom solid are equal
        }
        leftHit=false;
        rightHit=false;
    }

    //if top and bottom solid are equal, the height map is already set up
    if(topsolid!=bottomsolid){
        for(unsigned int i=0; i<=15; i++){
            hmapValue=0; //Reset hmap value for every new x value
            for(unsigned int ii=0; ii<=15; ii++){
                //origin is at the bottom
                //Check for alpha values starting from the bottom up
                if(topsolid < bottomsolid){
                    solid=(texture->GetPixelAlpha(i+coord.x, coord.GetBottom()-ii-1)==255);
                    if(solid){
                            hmapValue++;
                    }
                    else{
                        break;
                    }
                }
                //origin is at the Top
                //Check for alpha values starting from the top down
                else if (topsolid > bottomsolid){
                    solid=(texture->GetPixelAlpha(i+coord.x, coord.GetTop()+ii)==255);
                    if(solid){
                            hmapValue++;
                    }
                    else{
                        break;
                    }
                }
            }
            hHMap[i]=hmapValue;
        }
    }

    //if left and right solid are equal, the height map is already set up
    if(leftsolid != rightsolid){
        for(unsigned int i=0; i<=15; i++){
            hmapValue=0; //Reset hmap value for every new x value
            for(unsigned int ii=0; ii<=15; ii++){
                //origin is at the right
                //Check for alpha values starting from the right to the left
                if(leftsolid < rightsolid){
                    solid=(texture->GetPixelAlpha(coord.GetRight()-ii-1, i+coord.y)==255);
                    if(solid){
                            hmapValue++;
                    }
                    else{
                        break;
                    }
                }
                //origin is at the left
                //Check for alpha values starting from the left to right
                else{
                    solid=(texture->GetPixelAlpha(coord.GetLeft()+ii, i+coord.y)==255);
                    if(solid){
                            hmapValue++;
                    }
                    else{
                        break;
                    }
                }
            }
            vHMap[i]=hmapValue;
        }
    }

    RSC_Heightmap hmap(hHMap,vHMap);
    tileHMAPs[id]=hmap;
}

std::string TiledSet::GetTileProperty(GID id, const std::string& property) const{
	auto i = tileProperties.find(id);
	if ( i == tileProperties.end() ) {return "";}

	auto propertyIterator = i->second.find(property);
	if ( propertyIterator == i->second.end()){ return "";}

	return std::get<1>(propertyIterator->second);
}
