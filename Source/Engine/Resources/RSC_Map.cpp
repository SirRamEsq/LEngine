#include "RSC_Map.h"
#include "../Kernel.h"
#include "../Exceptions.h"

#include <stdlib.h>
#include <sstream>



void CopyPropertyMap(const PropertyMap& source, PropertyMap& destination){
    destination.clear();
    for(auto i=source.begin(); i!=source.end(); i++){
        destination[i->first]=i->second;
    }
}

GIDEnabled::GIDEnabled(GIDManager* man, GID first, GID last)
    : manager(man), firstGID(first), lastGID(last){
    if(firstGID>lastGID){ErrorLog::WriteToFile("[C++] GIDEnabled Constructor: lowest gid passed is greater than highest GID passed", ErrorLog::GenericLogFile);}

    manager->NewRange(first, last, this);
}

GIDEnabled::GIDEnabled(GIDManager* man)
    : manager(man){
    firstGID=0;
    lastGID=0;
}

GIDManager::Range GIDManager::NewRange(const GID& first, const GID& last, GIDEnabled* item){
    Range r(first, last);
    GIDItems[r]=item;
    return r;
}

GIDEnabled* GIDManager::GetItem (const GID& index) const {
    for(auto i=GIDItems.begin(); i!=GIDItems.end(); i++){
        if((i->first.low <= index) and (i->first.high >= index)){
            return i->second;
        }
    }
    return NULL;
}

////////////
//TiledSet//
////////////
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
        ErrorLog::WriteToFile(ss.str(), ErrorLog::GenericLogFile);
        initializationOK=false;
        return;
    }
    if(texture==NULL){ErrorLog::WriteToFile("[C++; TiledSet Constructor] Couldn't load texture from name " + textureName, ErrorLog::GenericLogFile);}

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
            throw LEngineFileException("Couldn't load texture" + textureName, ErrorLog::GenericLogFile);
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
        //ErrorLog::WriteToFile(ss.str());
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

/////////////////////
//TiledLayerGeneric//
/////////////////////

TiledLayerGeneric::TiledLayerGeneric(const unsigned int& tileW, const unsigned int& tileH, const std::string& name, const MAP_DEPTH& depth, const GIDManager* g, const L_TILED_LAYER_TYPE& type)
    : tileWidth(tileW), tileHeight(tileH), pixelWidth(tileW*16), pixelHeight(tileH*16), layerName(name), layerDepth(depth), GIDM(g), layerType(type){

}

bool TiledLayerGeneric::Ignore() const{
    std::string propertyName="IGNORE";
    if(PropertyExists(propertyName) == false){
        return false;
    }
    return (GetPropertyValue(propertyName) != "false");
}

std::string TiledLayerGeneric::GetPropertyValue(const std::string& propertyName) const{
    auto i = properties.find(propertyName);
    if(i == properties.end()){
        return "";
    }

    return std::get<1>(i->second);
}

bool TiledLayerGeneric::PropertyExists(const std::string& propertyName) const{
    return (properties.find(propertyName) != properties.end());
}

////////////////////
//TiledObjectLayer//
////////////////////

TiledObjectLayer::TiledObjectLayer(const unsigned int& w, const unsigned int& h, const std::string& name, const MAP_DEPTH& depth, const GIDManager* g)
    : TiledLayerGeneric(w, h, name, depth, g, LAYER_OBJECT){

}

TiledObjectLayer::TiledObjectLayer(const TiledObjectLayer& rhs, const GIDManager* g)
    : TiledLayerGeneric(rhs.tileWidth, rhs.tileHeight, rhs.layerName, rhs.GetDepth(), g, rhs.layerType){
    for(auto i=rhs.objects.begin(); i!=rhs.objects.end(); i++){
        objects[i->first] = i->second;
    }
    layerFlags = rhs.layerFlags;
    layerDepth = rhs.layerDepth;
    layerOpacity = rhs.layerOpacity;
    layerVisible = rhs.layerVisible;
    CopyPropertyMap(rhs.properties, properties);
}

//////////////////
//TiledTileLayer//
//////////////////

void TiledTileLayer::InitializeMap(){
    //Build 2D array
    for(int x=0; x<tileWidth; x++){
        std::vector<GID> g;
        data2D.push_back(g);
        for(int y=0; y<tileHeight; y++){
            data2D.back().push_back(0);
        }
    }
}

TiledTileLayer::TiledTileLayer(const unsigned int& w, const unsigned int& h, const std::string& name, const MAP_DEPTH& depth, const GIDManager* g)
    : TiledLayerGeneric(w, h, name, depth, g, LAYER_TILE){
    InitializeMap();
}

TiledTileLayer::TiledTileLayer(const TiledTileLayer& rhs, const GIDManager* g)
    : TiledLayerGeneric(rhs.tileWidth, rhs.tileHeight, rhs.layerName, rhs.GetDepth(), g, rhs.layerType){

    for(int x=0; x<rhs.tileWidth; x++){
        std::vector<GID> g;
        data2D.push_back(g);
        for(int y=0; y<rhs.tileHeight; y++){
            data2D.back().push_back(31337);
        }
    }
    for(int i=0; i<rhs.tileWidth; i++){
        for(int ii=0; ii<rhs.tileHeight; ii++){
            data2D[i][ii] = rhs.data2D[i][ii];
        }
    }

    animatedRefreshRate = rhs.animatedRefreshRate;
    friction = rhs.friction;
    tileSet = rhs.tileSet;

    layerFlags = rhs.layerFlags;
    layerDepth = rhs.layerDepth;
    layerOpacity = rhs.layerOpacity;
    layerVisible = rhs.layerVisible;
    CopyPropertyMap(rhs.properties, properties);
}

bool TiledTileLayer::IsValidXY(unsigned int x, unsigned int y) const{
    if( (x >= 0) && (y >= 0) && (x < tileWidth) && (y < tileHeight) ){
		return true;
	}
	return false;
}

GID TiledTileLayer::GetGID(unsigned int x, unsigned int y) const {
    if(IsValidXY(x,y)){
        return data2D[x][y];
    }
    return 0;
}
void TiledTileLayer::SetGID(unsigned int x, unsigned int y, GID id) const {
    if(IsValidXY(x,y)){
        data2D[x][y] = id;
    }
}
unsigned int TiledTileLayer::GetTile(unsigned int x, unsigned int y) const {
    if(IsValidXY(x,y)){
        return data2D[x][y];
    }
	return 0;
}
void TiledTileLayer::SetTile(unsigned int x, unsigned int y, unsigned int id) const {
    if(IsValidXY(x,y)){
        data2D[x][y] = id;
    }
}

void TiledTileLayer::UpdateRenderArea(CRect area) const {
    updatedAreas.push_back(area);
}


bool TiledTileLayer::HasTile(unsigned int x, unsigned int y) const{
    return (GetGID(x,y)!=0);
}

bool TiledTileLayer::IsDestructible() const {
    return ((layerFlags&TF_destructable)==TF_destructable);
}

bool TiledTileLayer::IsSolid() const {
    return ((layerFlags&TF_solid)==TF_solid);
}

bool TiledTileLayer::UsesHMaps() const {
    return ((layerFlags&TF_useHMap)==TF_useHMap);
}

float TiledTileLayer::GetFriction() const {
    return friction;
}

///////////////////
//TiledImageLayer//
///////////////////

TiledImageLayer::TiledImageLayer(const unsigned int& pixelW, const unsigned int& pixelH, const std::string& name, const MAP_DEPTH& depth, const GIDManager* g, const RSC_Texture* tex)
    : TiledLayerGeneric(pixelW/16, pixelH/16, name, depth, g, LAYER_IMAGE), texture(tex){

}

TiledImageLayer::TiledImageLayer(const TiledImageLayer& rhs, const GIDManager* g)
    : TiledLayerGeneric(rhs.tileWidth, rhs.tileHeight, rhs.layerName, rhs.GetDepth(), g, rhs.layerType), texture(rhs.texture){
    layerFlags = rhs.layerFlags;
    layerDepth = rhs.layerDepth;
    layerOpacity = rhs.layerOpacity;
    layerVisible = rhs.layerVisible;
    CopyPropertyMap(rhs.properties, properties);
    offset = rhs.offset;
    paralax = rhs.paralax;
}

/////////////
//TiledData//
/////////////

TiledData::TiledData(const unsigned int& tWidth, const unsigned int& tHeight)
    :tileWidth(tWidth), tileHeight(tHeight), width(tWidth*16), height(tHeight*16){

}

TiledData::TiledData(const TiledData& rhs)
    :tileWidth(rhs.tileWidth), tileHeight(rhs.tileHeight), width(rhs.width), height(rhs.height){

    bgColor = rhs.bgColor;
    gid = rhs.gid;

    for (auto i=rhs.tiledTileSets.begin(); i!= rhs.tiledTileSets.end(); i++){
        std::unique_ptr<TiledSet> newSet( new TiledSet(*(i->get()), &gid) );

        AddTileSet( std::unique_ptr<TiledSet>(newSet.release()) ) ;
    }
    for (auto i=rhs.tiledImageLayers.begin(); i!= rhs.tiledImageLayers.end(); i++){
        std::unique_ptr<TiledLayerGeneric> layer( new TiledImageLayer(*(i->get()), &gid) );

        AddLayer( std::unique_ptr<TiledLayerGeneric> (layer.release()) );
    }
    for (auto i=rhs.tiledTileLayers.begin(); i!= rhs.tiledTileLayers.end(); i++){
        std::unique_ptr<TiledLayerGeneric> layer( new TiledTileLayer(*(i->get()), &gid));

        AddLayer( std::unique_ptr<TiledLayerGeneric> (layer.release()) );
    }
    for (auto i=rhs.tiledObjectLayers.begin(); i!= rhs.tiledObjectLayers.end(); i++){
        std::unique_ptr<TiledLayerGeneric> layer( new TiledObjectLayer(*(i->get()), &gid) );

        AddLayer( std::unique_ptr<TiledLayerGeneric> (layer.release()) );
    }

    CopyPropertyMap(rhs.properties, properties);
}

TiledData::~TiledData(){

}

bool TiledData::AddTileSet (std::unique_ptr<TiledSet> tileSet){

    tiledSets[tileSet->name] = tileSet.get();
    tiledTileSets.push_back( std::unique_ptr<TiledSet>(tileSet.release()) );
    return true;
}

bool TiledData::AddLayer(std::unique_ptr<TiledLayerGeneric> layer){
    MAP_DEPTH depth = layer->GetDepth();
    auto layerType = layer->layerType;

    //Make sure depth isn't already taken if the layer is supposed to be rendered
    if( (layerType == LAYER_IMAGE)or(layerType == LAYER_TILE) ){
        if(tiledRenderableLayers.find(depth)!=tiledRenderableLayers.end()){
            //if depth already exists, return 0 and implicitly delete layer via smart pointer
            std::stringstream ss;
            ss << "Couldn't add TileLayer named: " << layer->layerName
            << "\n    With Depth of: " << depth
            << "\n    Depth already taken";
            ErrorLog::WriteToFile(ss.str());
            return false;
        }

        //Ignore layer if ignore flag is set
        if(layer->Ignore() == true){
            return true;
        }

        //Add layer to map if depth isn't already taken
        tiledLayers[layer->layerName] = layer.get();
        tiledRenderableLayers[depth]  = layer.get();

        if(layerType == LAYER_IMAGE){
            tiledImageLayers.push_back( std::unique_ptr<TiledImageLayer>((TiledImageLayer*)layer.release()) );
        }
        else if (layerType == LAYER_TILE){
            tiledTileLayers.push_back( std::unique_ptr<TiledTileLayer>((TiledTileLayer*)layer.release()) );
        }
    }
    else if(layerType == LAYER_OBJECT){
        tiledLayers[layer->layerName]=layer.get();
        tiledObjectLayers.push_back( std::unique_ptr<TiledObjectLayer>((TiledObjectLayer*)layer.release()) );
    }



    return true;
}


RSC_Map::RSC_Map(){

}
RSC_Map::~RSC_Map(){

}

std::string TiledTileLayer::GetTileProperty(GID id, const std::string& property) const{
	if(id == 0){return "";}
	return tileSet->GetTileProperty(id, property);
}
////////
//RSC_MapImpl//
////////

RSC_MapImpl::RSC_MapImpl(std::unique_ptr<TiledData> td){
    tiledData.reset();
    tiledData = std::move(td);
}

EID RSC_MapImpl::GetEIDFromName(const std::string& name)const{
    return 0;
}


std::string RSC_MapImpl::GetMapName() const {return mMapName;}
TiledData* RSC_MapImpl::GetTiledData(){return tiledData.get();}

RSC_MapImpl::RSC_MapImpl(const RSC_MapImpl& rhs){
    mMapName            = rhs.mMapName;
    firstGID            = rhs.firstGID;

    tiledData = make_unique<TiledData> (*rhs.tiledData.get());
}

 RSC_MapImpl::~RSC_MapImpl(){
 }


int RSC_MapImpl::GetWidthTiles() const{
    return tiledData->tileWidth;
}
int RSC_MapImpl::GetHeightTiles() const{
    return tiledData->tileWidth;
}
int RSC_MapImpl::GetWidthPixels() const{
    return tiledData->width;
}
int RSC_MapImpl::GetHeightPixels() const{
    return tiledData->height;
}

const TiledTileLayer* RSC_MapImpl::GetTileLayerCollision(int x, int y, bool areTheseTileCoords) const {
    //Return first tile layer collided with
    if(tiledData.get()->tiledTileLayers.empty()){
        return NULL;
    }

    int xt=x;
    int yt=y;
    if(!areTheseTileCoords){
        CoordToGrid(xt,yt);
    }

    auto i=tiledData.get()->tiledTileLayers.begin();
    if(x>=i->get()->data2D.size())      {return NULL;}
    if(y>=i->get()->data2D[xt].size())  {return NULL;}
    if(x<0){return NULL;}
    if(y<0){return NULL;}

    GID id=0;
    for(; i!=tiledData.get()->tiledTileLayers.end(); i++){
        if((i->get()->layerFlags & TF_solid)==TF_solid){
            id=i->get()->data2D[xt][yt];
            if(id!=0){
                return i->get();
            }
        }
    }
    return NULL;
}

//Pass a map with keys to get their respective values from the xml file
//Pass an empty map to get everything
void TiledData::TMXLoadAttributes(rapidxml::xml_node<>* rootAttributeNode, AttributeMap& attributes){
    using namespace rapidxml;
    std::string key, value, type;
    AttributeMap::iterator i;
    bool emptyMap=attributes.empty(); //returns true if map is empty

    //Iterate through each attribute in the given node
    for (xml_attribute<> *attr = rootAttributeNode->first_attribute(); attr; attr = attr->next_attribute()){
        key=attr->name();
        value=attr->value();
        //ErrorLog::WriteToFile("Key is ");
        //ErrorLog::WriteToFile(key);

        //If the attribute map passed in is empty, simply fill it with every attribute found
        if(emptyMap){
            attributes[key]=Attribute(value,NULL);
        }
        //If the attribute map is not empty, match each found key in the node with it's corresponding value in the map
        else{
            i=attributes.find(key);
            if(i!=attributes.end()){
                type=i->second.first;
                TMXProcessType(type, value, i->second.second);
            }
        }
    }
}

void TiledData::TMXLoadAttributesFromProperties(const PropertyMap* properties, AttributeMap& attributes){
    std::string data;
    std::string type;
    std::string name;
    for(auto i = properties->begin(); i!=properties->end(); i++){
        name=i->first;
        auto attributeIterator=attributes.find(name);

        if(attributeIterator!=attributes.end()){
            type=attributeIterator->second.first;
            data=i->second.second;
            TMXProcessType(type, data, attributeIterator->second.second);
        }
    }
}
//root node passed should point to <properties> tag
void TiledData::TMXLoadProperties(rapidxml::xml_node<>* rootPropertyNode, PropertyMap& properties){
    using namespace rapidxml;
    xml_node<>* propertyNode=rootPropertyNode->first_node(); //pointing to property

    std::string nameString, valueString, typeString;

    //properties are siblings, not attributes
    xml_attribute<>* attributeName;
    xml_attribute<>* attributeType;
    xml_attribute<>* attributeValue;
    for(;propertyNode; propertyNode=propertyNode->next_sibling()){
        typeString="string";
        nameString="";
        valueString="";
        attributeName   = propertyNode->first_attribute("name");
        attributeType   = propertyNode->first_attribute("type");
        attributeValue  = propertyNode->first_attribute("value");

        if(attributeName)  { nameString  = attributeName-> value(); }
        if(attributeType)  { typeString  = attributeType-> value(); }
        if(attributeValue) { valueString = attributeValue->value(); }

        properties[nameString]=StringPair(typeString, valueString);
    }
}

void TiledData::TMXProcessEventListeners(std::string& listenersString, std::vector<EID>& listeners){
    if(listenersString!=""){
        std::vector<const char *> subStrings;
        char* tempString=NULL;
        char* propertyCString= new char [listenersString.size()+1];
        strcpy(propertyCString,listenersString.c_str());

        tempString=strtok (propertyCString,", ");
        if(tempString!=NULL){
            subStrings.push_back(tempString);
            while (true){
                tempString=strtok (NULL,", ");
                if(tempString==NULL){break;}
                subStrings.push_back(tempString);
            }
        }
        else{subStrings.push_back(listenersString.c_str());}


        //Have list of strings, now just set up the correct listen IDs for the object
        int tempID;

        std::vector<const char *>::iterator i=subStrings.begin();
        for(;i!=subStrings.end(); i++){
            tempID=strtol((*i), NULL, 10);

            listeners.push_back(tempID);
        }

        delete [] propertyCString;
    }
}

void TiledData::TMXProcessType(std::string& type, std::string& value, void* data){
    if      (type=="string")    {
        *((std::string*)(data))=value;
    }
    else if (type=="int")      {
        *((int*)(data))=strtol (value.c_str(), NULL, 10);
    }
    else if (type=="unsigned long")      {
        *((unsigned long*)(data))=strtol (value.c_str(), NULL, 10);
    }
    else if (type=="unsigned int")      {
        *((unsigned int*)(data))=strtol (value.c_str(), NULL, 10);
    }
    else if (type=="long")      {
        *((long*)(data))=strtol (value.c_str(), NULL, 10);
    }
    else if (type=="EID")      {
        *((EID*)(data))=strtol (value.c_str(), NULL, 10);
    }
    else if (type=="GID")      {
        *((GID*)(data))=strtol (value.c_str(), NULL, 10);
    }
    else if (type=="float")      {
        *((float*)(data))=atof  (value.c_str());
    }
    else if (type=="double")      {
        *((double*)(data))=atof  (value.c_str());
    }
    else if (type=="bool")      {
        *((bool*)(data))=(value=="true");
    }
}

std::string     RSC_MapImpl::GetProperty (const std::string& property) const{
    tiledData->GetProperty(property);
}

std::string     TiledData::GetProperty (const std::string& property){
    auto i = properties.find(property);
    if(i == properties.end()){
        return "";
    }
    return i->second.second;
}

TiledTileLayer* RSC_MapImpl::GetTileLayer (const std::string& property){
    tiledData->GetTileLayer(property);
}

TiledTileLayer* TiledData::GetTileLayer(const std::string& name){
    TiledLayerGeneric* layer = tiledLayers[name];
    if(layer != NULL){
        if(layer->layerType == LAYER_TILE){
            return (TiledTileLayer*)layer;
        }
    }
    return NULL;
}

std::unique_ptr<RSC_Map> RSC_MapImpl::LoadResource(const std::string& fname){
    std::unique_ptr<RSC_MapImpl> rscMap = NULL;
    try{
        std::string fullPath= "Resources/Maps/"+fname;
        auto data=LoadGenericFile(fullPath);
        if(data.get()->GetData()==NULL){
            throw LEngineFileException("Couldn't load RSC_MapImpl from path", fname);
        }

        try{
            auto tiledData = TiledData::LoadResourceFromTMX(fname, data.get()->GetData(), data.get()->length);
            rscMap = make_unique<RSC_MapImpl>( std::move(tiledData) ) ;
        }
        catch(RSC_Map::Exception e){
            ErrorLog::WriteToFile(e.what(), ErrorLog::GenericLogFile);
            throw e;
        }
    }
    catch(LEngineFileException e){
        ErrorLog::WriteToFile(e.what(), ErrorLog::GenericLogFile);
        throw e;
    }

    return rscMap;
}

std::unique_ptr<TiledData> TiledData::LoadResourceFromTMX(const std::string& TMXname, const char* dat, unsigned int fsize){
    //There are three primary data structures this function looks for
    //TileSets
    //TileLayers (Large 2D array that references TileSets)
    //ObjectLayers
    //Tilesets are always first in the xml file, then come the layers (object and tile) in order of depth

    std::string XML=std::string(dat,fsize);

    std::stringstream mapInitializeDebugMessage;
    mapInitializeDebugMessage << "[C++; RSC_MapImpl:LoadTMX] Filename is: " << TMXname;

    ErrorLog::WriteToFile(mapInitializeDebugMessage.str(), ErrorLog::GenericLogFile);
    using namespace rapidxml;
    xml_document<> doc;    // character type defaults to char
    AttributeMap attributes; //this will contain all of the attributes of a given part of the map as the function goes on
    //Convert const char to char

    //WILL CAUSE PROBLEMS IF std::string XML IS USED BEYOND THIS POINT
    doc.parse<0>((char*)(XML.c_str()));    // 0 means default parse flags

    GID id;

    std::string errorString;

    //Find Specific Node
    xml_node<> *node = doc.first_node("map");

    std::string testString;
    std::string valueString;

    //This is used for loading multiple maps that share the same tiled set;
    std::map<GIDManager::Range, GIDManager::Range> tiledToEngineGID;

    //Map Properties
    std::string bgColorString;
    unsigned int tilesWide, tilesHigh, sizeOfTileWidth, sizeOfTileHeight;
    attributes["backgroundcolor"]   = Attribute("string", &bgColorString);
    attributes["width"]             = Attribute("unsigned int", &tilesWide);
    attributes["height"]            = Attribute("unsigned int", &tilesHigh);
    attributes["tilewidth"]         = Attribute("unsigned int", &sizeOfTileWidth);
    attributes["tileheight"]        = Attribute("unsigned int", &sizeOfTileHeight);
    TiledData::TMXLoadAttributes(node, attributes);
    auto tiledData = make_unique<TiledData>(tilesWide, tilesHigh);

    if((sizeOfTileWidth != LENGINE_DEF_TILE_W) or (sizeOfTileHeight != LENGINE_DEF_TILE_H)){
        std::stringstream ss;
        ss << "Couldn't load map named: " << TMXname
        << "\n    Tile width and height are not 16 pixels";
        ErrorLog::WriteToFile(ss.str());
    }

    //Translate background color from string to int
    const char* pointer=valueString.c_str();
    pointer++; //Skip over the "#" character
    tiledData->bgColor  =strtol( pointer, NULL, 16 );//base 16
    pointer=NULL;

    node=node->first_node(); //point to the first child of <map>
    xml_node<> *subnode;

    //Main loop, gets all of <map> children
    for(; node!=0; node=node->next_sibling()){
        std::string nn(node->name());
        ErrorLog::WriteToFile(nn);

        if(nn=="properties"){ //Map properties; only one of these in the whole map file; specifies the global script and what entities it listens to
            std::string listenString="";
            std::string scriptString="";

            TiledData::TMXLoadProperties(node, tiledData->properties);

            attributes.clear();
            attributes["SCRIPT"]           = Attribute("string", &scriptString);
            attributes["LISTEN"]           = Attribute("string", &listenString);
            TMXLoadAttributesFromProperties(&tiledData->properties, attributes);

            if(scriptString==""){continue;}

            //globalScriptName=scriptString;
            //TiledData::TMXProcessEventListeners(listenString,mEventSources);
        }

        else if(nn=="tileset"){
            GID tilesetFirstGID;
            std::string tileSetSource=""; //If the tileset is an external file, store its path here

            attributes.clear();
            attributes["firstgid"]          = Attribute("GID",  &tilesetFirstGID);
            attributes["source"]            = Attribute("string", &tileSetSource  );
            TMXLoadAttributes(node, attributes);

            //If there is a 'source' attribute defined in the tileset node, then that means the tile set is an external file
            bool externalTilesetFile = (tileSetSource!="");
            xml_node<>* tileSetRootNode=node;
            xml_document<> tileSetDoc;
            std::unique_ptr<FileData> file;
            std::string xmlFile;
            if(externalTilesetFile){
                try{
                    //Some where in here memory may not be allocated properly, leading the the node name corruption in the tileset loader
                    std::stringstream externalTilesetFilePath;
                    externalTilesetFilePath << "/Resources/Maps/" << tileSetSource;

                    file.reset( LoadGenericFile(externalTilesetFilePath.str()).release() );

                    if(file.get()->length==0){ErrorLog::WriteToFile("File Length is null", ErrorLog::GenericLogFile);}
                    if(file.get()->GetData()==NULL){ErrorLog::WriteToFile("Data is null", ErrorLog::GenericLogFile);}

                    xmlFile = std::string(file->GetData(),file->length);
                    tileSetDoc.parse<0>((char*)xmlFile.c_str());

                    tileSetRootNode = tileSetDoc.first_node("tileset");
                }
                catch(rapidxml::parse_error e){
                    std::stringstream ss;
                    ss << "[C++; RSC_MapImpl::LoadTMX] TileSetDoc threw a rapidxml::parse error" << "\n    What is: " << e.what();
                    ErrorLog::WriteToFile(ss.str(), ErrorLog::GenericLogFile);

                }
                catch(LEngineFileException e){
                    std::stringstream ss;
                    ss << "[C++; RSC_MapImpl::LoadTMX] TileSetDoc threw a fileException error" << "\n    What is: " << e.what();
                    ErrorLog::WriteToFile(ss.str(), ErrorLog::GenericLogFile);
                }
            }
            //External Tilesets don't have their first gid included in their file, only the actual map assigns gids
            //that's why the first gid is passed as a separate value here
            tiledData->AddTileSet(TMXLoadTiledSet(tileSetRootNode, tilesetFirstGID, tiledData->gid));
        }

        else if(nn=="layer"){
            tiledData->AddLayer(TMXLoadTiledTileLayer(node, tiledData->gid));
        }

        else if(nn=="objectgroup"){
            tiledData->AddLayer(TMXLoadTiledObjectLayer(node, tiledData.get()));
        }

        else if(nn=="imagelayer"){
            tiledData->AddLayer(TMXLoadTiledImageLayer(node, tiledData->gid));
        }

    }


    //Iterate through every tile in every tile layer, updating its range
    GID oldID=0;
    const GIDManager::Range* engineRange, *localRange;
    auto layerIt=tiledData->tiledTileLayers.begin();
    for(;layerIt!=tiledData->tiledTileLayers.end();layerIt++){

        auto itX=(layerIt->get())->data2D.begin();
        for(;itX!=(layerIt->get())->data2D.end(); itX++){

            auto itY=itX->begin();
            for(;itY!=itX->end();itY++){

                //Get the old GID that the tile expects
                oldID=(*itY);
                if(oldID==0){continue;}

                //Get the corresponding GID that the engine has
                auto i=tiledToEngineGID.begin();
                engineRange=NULL;
                localRange=NULL;

                for(; i!=tiledToEngineGID.end(); i++){
                    //if the oldId is foudn within this old range.
                    if((i->first.low <= oldID) and (i->first.high >= oldID)){
                        engineRange=&(i->second);
                        localRange=&(i->first);
                        break;
                    }
                }
                if(engineRange==NULL){
                    //ErrorLog::WriteToFile("Oh Noes");
                    break;
                }

                GID localRangeDifference=(oldID-localRange->low);
                (*itY)=engineRange->low + localRangeDifference ;
            }
        }
    }
    //GIDManager::SetHighestGID(finalGID);
    return tiledData;
}

std::unique_ptr<TiledTileLayer> TiledData::TMXLoadTiledTileLayer (rapidxml::xml_node<>* rootNode, const GIDManager& gidManager){
    unsigned int animationRate=0; //if this variable stays zero, then there won't be any tile animations
    int depth;
    unsigned int width;
    unsigned int height;
    std::string name;
    float alpha=1.0f;

    std::string testString, valueString;

    AttributeMap attributes;

    attributes.clear();
    attributes["name"]   = Attribute("string", &name);
    attributes["width"]  = Attribute("unsigned int",    &width);
    attributes["height"] = Attribute("unsigned int",    &height);
    attributes["opacity"]= Attribute("float", &alpha);
    TMXLoadAttributes(rootNode, attributes);

    rapidxml::xml_node<>* subnode=rootNode->first_node();
    testString=subnode->name();

    bool propertyCollision=false;
    bool propertyDestructible=false;
    bool propertyHMap=false;
    float friction;
    std::map<std::string, std::string>  extraProperties;


    PropertyMap properties;
    TMXLoadProperties(subnode, properties);

    attributes.clear();
    attributes["DEPTH"]         = Attribute("int",  &depth);
    attributes["FRICTION"]      = Attribute("float",&friction);
    attributes["DESTRUCTIBLE"]  = Attribute("bool", &propertyDestructible);
    attributes["COLLISION"]     = Attribute("bool", &propertyCollision);
    attributes["USEHMAP"]       = Attribute("bool", &propertyHMap);
    attributes["ANIMATIONSPEED"]= Attribute("unsigned int",  &animationRate);
    TMXLoadAttributesFromProperties(&properties, attributes); //store unspecified properties in tileLayer->extraproperties

    std::unique_ptr<TiledTileLayer> tileLayer(new TiledTileLayer(width, height, name, depth, &gidManager));
    tileLayer->layerFlags=0;
    tileLayer->friction=friction;
    tileLayer->layerOpacity=alpha;
    CopyPropertyMap(properties, tileLayer->properties);

    if(propertyCollision)   {tileLayer->layerFlags = tileLayer->layerFlags | TF_solid;       }
    if(propertyDestructible){tileLayer->layerFlags = tileLayer->layerFlags | TF_destructable;}
    if(propertyHMap)        {tileLayer->layerFlags = tileLayer->layerFlags | TF_useHMap;     }

    subnode=subnode->next_sibling();
    std::vector<GID> data;
    GID id=0;
    for(subnode=subnode->first_node(); subnode; subnode=subnode->next_sibling()){
        valueString=subnode->first_attribute("gid")->value();
        //ErrorLog::WriteToFile("ValueString is: ", valueString);
        id=strtol( valueString.c_str(), NULL, 10);

        data.push_back(id);
    }
    int tilesWide=tileLayer->tileWidth;
    int tilesHigh=tileLayer->tileHeight;

    //load the correct data into 2dmap
    int x=0;
    int y=0;

    //Want to figure out what tileset this layer uses
    //All tilesets should be loaded by this point
    //This function will simply select the first tile set that it sees.
    tileLayer->tileSet=NULL;
    for(unsigned int i=0; i<data.size(); i++){
        if(tileLayer->tileSet==NULL){
            if(data[i]!=0){
                tileLayer->tileSet = ((TiledSet*)gidManager.GetItem(data[i]));
            }
        }
        tileLayer->data2D[x][y]=data[i];
        x++;
        if(x>=tilesWide){
            x=0;y++;
        }
    }
    tileLayer->animatedRefreshRate=animationRate; //if zero, no animation will occur
    //ErrorLog::WriteToFile(TMXname, ErrorLog::GenericLogFile);

    return tileLayer;
}

std::unique_ptr<TiledObjectLayer> TiledData::TMXLoadTiledObjectLayer (rapidxml::xml_node<>* rootNode, TiledData* tiledData){
    AttributeMap attributes;
    std::string objectLayerName=rootNode->first_attribute()->value();

    //Object Layer properties
    rapidxml::xml_node<>* subnodeProperties=rootNode->first_node();
    PropertyMap properties;
    //Load properties into layer
    TMXLoadProperties(subnodeProperties, properties);

    //Extract expected attributes
    int depth=0;
    attributes.clear();
    attributes["DEPTH"] = Attribute("int", &depth);
    TMXLoadAttributesFromProperties(&properties, attributes);

    std::unique_ptr<TiledObjectLayer> objectLayer = make_unique<TiledObjectLayer>(0,0, objectLayerName, depth, &tiledData->gid);
    objectLayer->properties=properties;

    //Siblings of <properties> are actual objects
    for(auto subnodeObject=subnodeProperties->next_sibling(); subnodeObject!=0; subnodeObject=subnodeObject->next_sibling()){
        int objID, objX, objY, objWidth, objHeight;
        std::string objName, objType;

        attributes.clear();
        attributes["id"]        = Attribute("int",      &objID);
        attributes["x"]         = Attribute("int",      &objX);
        attributes["y"]         = Attribute("int",      &objY);
        attributes["width"]     = Attribute("int",      &objWidth);
        attributes["height"]    = Attribute("int",      &objHeight);
        attributes["type"]      = Attribute("string",   &objType);
        attributes["name"]      = Attribute("string",   &objName);
        TMXLoadAttributes(subnodeObject, attributes);

        //create new object
        objectLayer->objects[objID]=TiledObject();
        TiledObject& newObj=(objectLayer->objects[objID]);

        //assign values
        newObj.script="";
        newObj.name=objName;
        newObj.type=objType;
        newObj.extraData=NULL;
        newObj.x=objX;
        newObj.y=objY;
        newObj.w=objWidth;
        newObj.h=objHeight;
        newObj.tiledID=objID;

        //Get pointer to node containing the new object's properties
        rapidxml::xml_node<>* objectPropertiesNode = subnodeObject->first_node();

        if(objectPropertiesNode!=NULL){
            std::string name;
            std::string value;
            std::string testString, valueString;

            //Variables to store property data that needs processed
            std::string eventString="";
            std::string listenString="";
            std::string type="";
            int eventNum;
            newObj.useEntrance=false;
            newObj.parent=0;

            //User defined properties for this particular object
            //Going to read this raw, without TMX hepler functions
            //more efficent, more contorl.
            rapidxml::xml_node<> *objectPropertyNode=objectPropertiesNode->first_node();
            for(; objectPropertyNode!=0; objectPropertyNode=objectPropertyNode->next_sibling()){
                type="string";

                //Get name, value, type
                for (rapidxml::xml_attribute<> *attr = objectPropertyNode->first_attribute(); attr; attr = attr->next_attribute()){
                    testString=attr->name();
                    valueString=attr->value();

                    if(testString=="name")      {name=valueString;}
                    else if(testString=="type") {type=valueString;}
                    else if(testString=="value"){value=valueString;}
                }

                //Assign property to correct variable
                if     (name=="SCRIPT")       {newObj.script=value;}
                if     (name=="LIGHT")        {if(valueString=="true"){newObj.light=true;}else{newObj.light=false;}}
                else if(name=="ID")           {eventNum =strtol(value.c_str(), NULL, 10);}
                else if(name=="MAP")          {eventString=value;}
                else if(name=="USE_ENTRANCE") {if(valueString=="true"){newObj.useEntrance=true;}}
                else if(name=="LISTEN")       {listenString=value;}
                else if(name=="PARENT")       {newObj.parent=strtol(value.c_str(), NULL, 10);}

                //If not what the engine expected, put in correct container
                else{
                    if(type=="string"){
                        newObj.stringProperties[name]=value;
                    }
                    else if(type=="bool"){
                        newObj.boolProperties[name]= (value=="true");
                    }
                    else if(type=="int"){
                        newObj.intProperties[name]=strtol(value.c_str(), NULL, 10);
                    }
                    else if(type=="float"){
                        newObj.floatProperties[name]=atof(value.c_str());
                    }
                }
            } //End for loop, go to next sibling (Next property)

            TMXProcessEventListeners(listenString,newObj.eventSources);

            //Process more complicated properties
            ErrorLog::WriteToFile(newObj.type, ErrorLog::GenericLogFile);
            if(newObj.type!=""){
                if(newObj.type==global_TiledStrings[TILED_EVT_MAP_ENTRANCE]){
                    MapEntrance e;
                    e.mPosition.x=newObj.x;
                    e.mPosition.y=newObj.y;
                    e.mPosition.w=newObj.w;
                    e.mPosition.h=newObj.h;
                    e.mEntranceID=eventNum;
                    tiledData->mMapEntrances[eventNum]=e;
                    newObj.extraData=&tiledData->mMapEntrances[eventNum];
                }
                else if(newObj.type==global_TiledStrings[TILED_EVT_MAP_EXIT]){
                    MapExit e;
                    e.mPosition.x=newObj.x;
                    e.mPosition.y=newObj.y;
                    e.mPosition.w=newObj.w;
                    e.mPosition.h=newObj.h;
                    e.mEntranceID=eventNum;
                    e.mMapName=eventString;
                    tiledData->mMapExits[eventNum]=e;
                    newObj.extraData=&tiledData->mMapExits[eventNum];
                }
                else if(newObj.type==global_TiledStrings[TILED_CAMERA]){
                    if(newObj.boolProperties["MAIN_CAMERA"]==true){
                        newObj.flags= (newObj.flags | TILED_OBJECT_IS_MAIN_CAMERA);
                    }
                }
            }
        }
    }
    return objectLayer;
}

std::unique_ptr<TiledImageLayer> TiledData::TMXLoadTiledImageLayer(rapidxml::xml_node<>* rootNode, const GIDManager& gidManager){
    rapidxml::xml_node<> *subNodeImage=rootNode->first_node();

    std::string name;
    AttributeMap attributes;
    attributes["name"]              = Attribute("string",     &name         );
    TMXLoadAttributes(rootNode, attributes);

    std::string texturePath;
    int w, h;
    attributes.clear();
    attributes["source"]             = Attribute("string",    &texturePath  );
    attributes["width"]              = Attribute("int",       &w            );
    attributes["height"]             = Attribute("int",       &h            );
    TMXLoadAttributes(subNodeImage, attributes);

    rapidxml::xml_node<>* subNodeProperties=subNodeImage->next_sibling();
    int depth=0;

    const RSC_Texture* texture= K_TextureMan.GetItem(texturePath);
    if(texture==NULL){
        K_TextureMan.LoadItem(texturePath, texturePath);
        texture=K_TextureMan.GetItem(texturePath);
        if(texture==NULL){
            std::stringstream ss;
            ss << "Couldn't load texture for Image Layer " << texturePath;
            ErrorLog::WriteToFile(ss.str());
            return NULL;
        }
    }
    std::unique_ptr<TiledImageLayer> imageLayer (new TiledImageLayer(w,h,name,depth, &gidManager, texture));

    float paralaxX, paralaxY;
    bool repeatX, repeatY;
    attributes.clear();
    attributes["paralaxX"]            = Attribute("float",    &paralaxX      );
    attributes["paralaxY"]            = Attribute("float",    &paralaxY      );
    attributes["repeatX"]             = Attribute("bool",     &repeatX       );
    attributes["repeatY"]             = Attribute("bool",     &repeatY       );
    TMXLoadProperties(subNodeProperties,imageLayer->properties);
    TMXLoadAttributesFromProperties(&imageLayer->properties, attributes);



    return imageLayer;
}

std::unique_ptr<TiledSet> TiledData::TMXLoadTiledSet(rapidxml::xml_node<>* tiledSetRootNode, const GID& firstGID, GIDManager& gidManager){
/*XML Tile property structure goes like this;
    <tileset>
        <image source="Images/GrassTerrain.png" trans="ff00ff" width="256" height="256"/>
        <tile ID="44">
            <properties>
                <property name="IMG_LENGTH" type="int" value="3"/>
                ...
                <property name="IMG_SPEED" type="int" value="10"/>
            </properties>
        </tile>
        <tile ID="45">
            ...
        </tile>
        ...
    </tileset>
*/
    rapidxml::xml_node<> *subNodeImage=tiledSetRootNode->first_node();
    //Tileset attributes
    GID tilesetFirstGID=firstGID;
    std::string name="";

    unsigned int tileWidth;
    unsigned int tileHeight;

    AttributeMap attributes;
    attributes["name"]              = Attribute("string",       &name           );
    attributes["tilewidth"]         = Attribute("unsigned int", &tileWidth);
    attributes["tileheight"]        = Attribute("unsigned int", &tileHeight);
    TMXLoadAttributes(tiledSetRootNode, attributes);

    //IMAGE Attributes
    std::string transparentColor="ff00ff";
    std::string textureName;

    attributes.clear();
    attributes["source"]        = Attribute("string",   &textureName);
    attributes["trans"]         = Attribute("string",   &transparentColor);
    //attributes["width"]         = Attribute("unsigned long",      &ts->imageWidth);
    //attributes["height"]        = Attribute("unsigned long",      &ts->imageHeight);
    TMXLoadAttributes(subNodeImage, attributes);

    //Load tileset into engine and set firstGID
    auto returnSmartPonter = std::unique_ptr<TiledSet>(new TiledSet(name, textureName, tileWidth, tileHeight, tilesetFirstGID, &gidManager));

    TiledSet* ts=returnSmartPonter.get();
    ts->transparentColor=transparentColor;

    //Set up Tiled GID Range
    ts->SetFirstGID(tilesetFirstGID);
    ts->SetLastGID((ts->GetFirstGID() + ts->GetTilesTotal())-1);

    //Get Unique Tile Properties
    rapidxml::xml_node<> *subNodeTileRoot=subNodeImage->next_sibling(); //points toward <tile> tag which is sibling of <image>

    //Iterate through all of the <tile> tags
    GID tilePropertyID;
    std::string valueString;
    for(; subNodeTileRoot!=0; subNodeTileRoot=subNodeTileRoot->next_sibling()){
        //Get GID of tile that the properties are associated with
        //This GID is the value of the tile within the tileset, between 0 and (max number of tiles -1)
        valueString = subNodeTileRoot->first_attribute()->value();
        tilePropertyID=strtol( valueString.c_str(), NULL, 10);

        //Add tileset's starting GID to get the actual value of the tile.
        tilePropertyID+=tilesetFirstGID;

        rapidxml::xml_node<> *subNodeTileProperty = subNodeTileRoot->first_node(); //Points toward <properties>

        PropertyMap properties;
        TMXLoadProperties(subNodeTileProperty, properties);

        attributes.clear();
        //Store animation data in the tileAnimations data structure
        std::string spriteName = "";
        std::string animationName = "";
        attributes["SPRITE"]     = Attribute("string",      &spriteName);
        attributes["ANIMATION"]  = Attribute("string",      &animationName);
        //Store all other properties in the tileProperties data structure of type map<string, string>
        TMXLoadAttributesFromProperties(&properties, attributes);

        if(spriteName != ""){
            const RSC_Sprite* spr = K_SpriteMan.GetLoadItem(spriteName, spriteName);
            if(spr != NULL){
                const LAnimation* animation = spr->GetAnimation(animationName);
                if(animation != NULL){
                    /*
                    Only load if the animation was created via animation sequence
                    (Meaning that the animation frames are contiguous on the texture)
                    */
                    if(animation->loadTag == LOAD_TAG_ANIMATION_SEQUENCE){
                        ts->tileAnimations[tilePropertyID]   = animation;
                    }
                    else{
                        ErrorLog::WriteToFile("[C++] RSC_MapImpl::TMXLoadTiledSet; For TileSet " + name
                                              + ": Animation named " + animationName + " in Sprite " + spriteName
                                              + " cannot be loaded as it was not defined using an 'animationSequence' tag",
                                              ErrorLog::GenericLogFile);
                    }
                }
            }
        }
        ts->tileProperties[tilePropertyID]=properties;

    }

    return returnSmartPonter;
}
