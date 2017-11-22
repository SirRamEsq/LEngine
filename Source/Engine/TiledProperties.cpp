#include "TiledProperties.h"

namespace tiledProperties {
std::string DEPTH = "_DEPTH";
std::string IGNORE = "_IGNORE";
std::string SHADER_FRAG = "_SHADER_FRAG";
std::string SHADER_VERT = "_SHADER_VERT";
std::string SHADER_GEO = "_SHADER_GEO";

namespace tile {
std::string PREFAB_PREFIX = "Tile/";
std::string SOLID = "_SOLID";
std::string HMAP = "_HMAP";
std::string ANIMATION_SPEED = "_ANIMATION_SPEED";
}

namespace object {
std::string PREFAB_PREFIX = "Object/";
std::string SCRIPT = "_SCRIPT";
std::string PARENT = "_PARENT";
std::string USE_ENTRANCE = "_USE_ENTRANCE";
std::string ENTRANCE_ID = "_ENTRANCE_ID";
std::string LISTEN_ID = "_LISTEN_ID";
std::string LISTEN_TYPE = "_LISTEN_TYPE";
}

namespace image {
std::string PREFAB_PREFIX = "Image/";
std::string STRETCH_X = "_STRETCH_X";
std::string STRETCH_Y = "_STRETCH_Y";
std::string PARALLAX_X = "_PARALLAX_X";
std::string PARALLAX_Y = "_PARALLAX_Y";
std::string REPEAT_X = "_REPEAT_X";
std::string REPEAT_Y = "_REPEAT_Y";
}

namespace tSet{
std::string SPRITE = "_SPRITE";
std::string ANIMATION = "_ANIMATION";
}

}
