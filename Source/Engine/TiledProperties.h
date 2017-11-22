#ifndef LENGINE_TILED_PROPERTIES
#define LENGINE_TILED_PROPERTIES

/**
 * This headerfile contains all currently reserved Tiled property names
 * All reserved properties begin with an underscore and are in ALL CAPS
 */

#include <string>

/**
 * Properties that apply to all Tiled Layers.
 * 'TYPE' property can be set to use a prefab
 */
namespace tiledProperties {
/// [INT] How Deep into the screen a layer is (negative is closer to camera)
extern std::string DEPTH;

/// [BOOL] Engine will not Load this layer if true
extern std::string IGNORE;

/// [STRING] Name of shader to use for this layer
extern std::string SHADER;

/**
 * Properties that apply to all Tile Layers.
 * Setting a prefab will apply all prefab properties to the layer
 */
namespace tile {
extern std::string PREFAB_PREFIX;

/// [BOOL] Will register Collisions if true
extern std::string SOLID;
/// [BOOL] Will generate and use heightmaps for this layer
extern std::string HMAP;
/// [INT] how many frames to update before animations update
extern std::string ANIMATION_SPEED;
}

/**
 * Properties that apply to all Object Layers.
 * Setting a prefab will apply all prefab properties to all objects in the layer
 */
namespace object {
extern std::string PREFAB_PREFIX;

/// [STRING] List of comma-delimited scripts to be run in a specified order
extern std::string SCRIPT;
/// [INT] Specifies what TiledID this entity's parent is
extern std::string PARENT;
/// [BOOL] Whether or not this object uses an entrance
extern std::string USE_ENTRANCE;
/// [INT] This entity acts as an entrance; ID of this entrance
extern std::string ENTRANCE_ID;
/// [STRING] What TiledEntities to recieve LuaEvents from
extern std::string LISTEN_ID;
/// [STRING] What LuaEvent Descriptions to listen to
extern std::string LISTEN_TYPE;
}

/**
 * Properties that apply to all Image Layers.
 * Setting a prefab will load all the prefab properties into the layer
 */
namespace image {
extern std::string PREFAB_PREFIX;

/// [BOOL] If true, Image will be stretched from each end of the X-Axis (Will
/// not repeat on X-AXIS)
extern std::string STRETCH_X;
/// [BOOL] If true, Image will be stretched from each end of the Y-Axis (Will
/// not repeat on Y-Axis)
extern std::string STRETCH_Y;

/// [FLOAT] Parallax Value to scroll the screen by on the X-Axis
extern std::string PARALLAX_X;
/// [FLOAT] Parallax Value to scroll the screen by on the Y-Axis
extern std::string PARALLAX_Y;

/// [BOOL] If true, background will repeat along the X-Axis instead of being
/// stretched
extern std::string REPEAT_X;
/// [BOOL] If true, background will repeat along the Y-Axis instead of being
/// stretched
extern std::string REPEAT_Y;
}

/**
 * Properties that apply to all tileSets
 */
namespace tSet{
/// [STRING] Default sprite for a certain tile to have
extern std::string SPRITE;
/// [STRING] Default Animation for a certain tile to have
extern std::string ANIMATION;
}

}

#endif
