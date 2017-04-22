#ifndef L_MAP_LOADER
#define L_MAP_LOADER

#include "Errorlog.h"
#include "Defines.h"
#include "TileMap.h"
#include "Resources/LMap.h"

#include "physfs.h"

#include <string>

TileMap* LoadRMap(std::string fname);
TileMap* LoadLuaMap(LMap* lmap);

#endif
