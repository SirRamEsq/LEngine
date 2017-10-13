#include "LayerTile.h"

void TiledTileLayer::InitializeMap(){
    //Build 2D array
    for(int x = 0; x < tileWidth; x++){
        std::vector<GID> g;
        data2D.push_back(g);
        for(int y = 0; y < tileHeight; y++){
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

    for(int x = 0; x < rhs.tileWidth; x++){
        std::vector<GID> g;
        data2D.push_back(g);
        for(int y = 0; y < rhs.tileHeight; y++){
            data2D.back().push_back(31337);
        }
    }
    for(int i = 0; i < rhs.tileWidth; i++){
        for(int ii = 0; ii < rhs.tileHeight; ii++){
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

void TiledTileLayer::UpdateRenderArea(Rect area) const {
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

