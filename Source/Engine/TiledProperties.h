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
std::string DEPTH = "_DEPTH";

/// [BOOL] Engine will not Load this layer if true
std::string IGNORE = "_IGNORE";

/// [STRING] Name of shader to use for this layer
std::string SHADER = "_SHADER";

/**
 * Properties that apply to all Tile Layers.
 * Setting a prefab will apply all prefab properties to the layer
 */
namespace tile {
/// [BOOL] Will register Collisions if true
std::string SOLID = "_SOLID";
/// [BOOL] Will generate and use heightmaps for this layer
std::string HMAP = "_HMAP";
/// [INT] how many frames to update before animations update
std::string ANIMATION_SPEED = "_ANIMATION_SPEED";
}

/**
 * Properties that apply to all Object Layers.
 * Setting a prefab will apply all prefab properties to all objects in the layer
 */
namespace object {
/// [STRING] List of comma-delimited scripts to be run in a specified order
std::string SCRIPT = "_SCRIPT";
/// [INT] Specifies what TiledID this entity's parent is
std::string PARENT = "_PARENT";
/// [BOOL] Whether or not this object uses an entrance
std::string USE_ENTRANCE = "_USE_ENTRANCE";
/// [INT] This entity acts as an entrance; ID of this entrance
std::string ENTRANCE_ID = "_ENTRANCE_ID";
/// [STRING] What TiledEntities to recieve LuaEvents from
std::string LISTEN_ID = "_LISTEN_ID";
/// [STRING] What LuaEvent Descriptions to listen to
std::string LISTEN_TYPE = "_LISTEN_TYPE";
}

/**
 * Properties that apply to all Image Layers.
 * Setting a prefab will load all the prefab properties into the layer
 */
namespace image {
/// [BOOL] If true, Image will be stretched from each end of the X-Axis (Will
/// not repeat on X-AXIS)
std::string STRETCH_X = "_STRETCH_X";
/// [BOOL] If true, Image will be stretched from each end of the Y-Axis (Will
/// not repeat on Y-Axis)
std::string STRETCH_Y = "_STRETCH_Y";

/// [FLOAT] Parallax Value to scroll the screen by on the X-Axis
std::string PARALLAX_X = "_PARALLAX_X";
/// [FLOAT] Parallax Value to scroll the screen by on the Y-Axis
std::string PARALLAX_Y = "_PARALLAX_Y";

/// [BOOL] If true, background will repeat along the X-Axis instead of being
/// stretched
std::string REPEAT_X = "_REPEAT_X";
/// [BOOL] If true, background will repeat along the Y-Axis instead of being
/// stretched
std::string REPEAT_Y = "_REPEAT_Y";
}

/**
 * Properties that apply to all tileSets
 */
namespace tSet{
/// [STRING] Default sprite for a certain tile to have
std::string SPRITE = "_SPRITE";
/// [STRING] Default Animation for a certain tile to have
std::string ANIMATION = "_ANIMATION";
}

}

#endif
