#ifndef L_MAP
#define L_MAP

#include "rapidxml.hpp"
#include <string>
#include <map>
#include <vector>
#include <memory>

#include "RSC_Sprite.h"
#include "RSC_Heightmap.h"
//#include "../RenderManager.h"

//These Types are used as helpers in the TMX loading functions
//Attributes have a name and reference a variable to store data in
//They do not contain type information, you are expected to know what type they are
//and provide storage for their values
//These are used for built in variables in Tiled, the types of which are already known.
                //Type, Value
typedef std::pair<std::string, void*> Attribute;
                //<Name                  <Type, value> >
typedef std::map<std::string, Attribute > AttributeMap;


//Properties are made of three strings, a name, a type, and a value in string form
//Properties are used when you have no idea what data the user may send via Tiled
                    //Type      Value
typedef std::pair<std::string, std::string> StringPair;
                    //Name
typedef std::map<std::string, StringPair>   PropertyMap;

void CopyPropertyMap(const PropertyMap& source, PropertyMap& destination);

//Forward declares
class RSC_MapImpl;
class TiledData;
class GIDManager;

//coupling here
class StateManager;
class GameState;
class ComponentCollisionManager;

enum L_TILED_LAYER_TYPE{
    LAYER_GENERIC   =   0,
    LAYER_OBJECT    =   1,
    LAYER_IMAGE     =   2,
    LAYER_TILE      =   3
};

const std::string L_TILED_LAYER_TYPE_STRINGS[] = {
    "LAYER_GENERIC",
    "LAYER_OBJECT",
    "LAYER_IMAGE",
    "LAYER_TILE"
};

enum L_TILED_IMAGE_LAYER_FLAGS{
    IMG_LAYER_REPEAT_X  = 1,
    IMG_LAYER_REPEAT_Y  = 2
};

//The GID Manager is used to re use resources that the tilemaps need
//So far, this only includes tile sets. backgrounds may later also be GID Enabled
//Tiled objects do not need to be GID enabled, they are all unique and don't need to be looked up with a Global ID

typedef unsigned long int GID;

//A GID is a global identifier that is used to look up data that is relevant to a certain resource
//A GID enabled class is a class that makes use of a range of GID values

//Once a GID enabled item is created, it's expected to not be deleted until the manager is
class GIDEnabled{
    public:
        GIDEnabled(GIDManager* man, GID first, GID last);
        GIDEnabled(GIDManager* man                     );

        void ReconcileToFirst(){ if(firstGID>lastGID){lastGID=firstGID;} }
        void ReconcileToLast (){ if(lastGID<firstGID){firstGID=lastGID;} }

        GID GetFirstGID      () const {return firstGID;                  }
        GID GetLastGID       () const {return lastGID;                   }

        void SetFirstGID      (GID id){firstGID=id;ReconcileToFirst();   }
        void SetLastGID       (GID id){lastGID=id;ReconcileToLast();     }

        void IncrementFirstGID(GID id){firstGID+=id;ReconcileToFirst();  }
        void IncrementLastGID (GID id){lastGID+=id;ReconcileToLast();    }

    private:
        GID firstGID, lastGID;
        GIDManager* manager;
};

//This class keeps a record of what GID ranges belong to which objects
class GIDManager{
    public:
        class Range{
            public:
                Range(const GID& l, const GID& h) {high=h;low=l;}  // [low,high]
                Range()                           {high=0; low=100;}

                Range& operator=(const Range& r) {
                    if (this!=&r){  // Same object?
                        low=r.low;
                        high=r.high;
                    }
                    return *this;
                }

                bool operator<(const Range& r) const{
                    if (low < r.low){
                        assert(high < r.low); // sanity check
                        return true;
                    }
                    return false;
                }

                GID high, low;
        };

        Range       NewRange (const GID& first, const GID& last, GIDEnabled* item);
        GIDEnabled* GetItem  (const GID& index                                   ) const;

    private:
        //GIDEnabled items can be lookedup by Range
        std::map<Range,         GIDEnabled*> GIDItems;
};
//No more Global GID manager. Each RSC_Map will contain its own set of data to create a map
//The only reason they were shared before, was in order to share the same spriets
//Now that these classes don't create sprites, it no longer serves a purpose

//A tiled set defines what tiles are. Each tile set has a texture and also contains special properties of any tiles should have them.
//Each tile has a unique GID. Each tileSet has a Range of GIDs for each tile it defines
//It's up to the GID Manager to keep track of what ranges belong to which tilesets.
class TiledSet : public GIDEnabled{
    friend RSC_MapImpl;
    friend TiledData;

    public:
        TiledSet(const std::string& n, const std::string& tex, const unsigned int tileW, const unsigned int tileH, GID first, GIDManager* man);
        TiledSet(const TiledSet& rhs, GIDManager* g);
        void Init(GID first, GIDManager* man);

        void        LoadTexture                   ();
        void        LoadHeightMaps                (GID id);
        bool        ContainsTile                  (GID id) const;
        void        GetTextureCoordinatesFromGID  (GID id, float& left, float& right, float& top, float& bottom) const;
        const LAnimation* GetAnimationDataFromGID       (GID id) const;
        CRect       GetTextureRectFromGID         (GID id) const;

        RSC_Heightmap        GetHeightMap  (GID id) const {return tileHMAPs.find(id)->second; }
        int               GetTilesWide  ()       const {return tilesWide;     }
        int               GetTilesHigh  ()       const {return tilesHigh;     }
        int               GetTilesTotal ()       const {return tilesTotal;    }
        std::string       GetTextureName()       const {return textureName;   }
		std::string			GetTileProperty(GID id, const std::string& property) const;
        const RSC_Texture*   GetTexture    ()       const {return texture;       }

        std::string transparentColor;

        const unsigned int tileWidth;
        const unsigned int tileHeight;

        const std::string name;
        const std::string textureName;

    protected:
        std::map<GID, const LAnimation*> tileAnimations; //This can be directly accessed by RSC_Map (for the purpose of loading the animations in from a file)

    private:
        bool initializationOK;

        int tilesWide;
        int tilesHigh;
        int tilesTotal;

        std::map<GID, PropertyMap > tileProperties;

        const RSC_Texture* texture;

        //Only used if HMAP flags are set
        std::map<GID, RSC_Heightmap> tileHMAPs;
};

//A TiledTileLayer defines the make up of a layer of terrain from the TiledSet that it makes use of
//The TiledTileLayer is, at it's core, a 2d array of GID values from a single TileSet.
//In addition to the properties outlined in the TiledSet, the TiledTileLayer can override and add to those properties.

class TiledLayerGeneric{
    friend RSC_MapImpl;
    friend TiledData;
    public:
        TiledLayerGeneric(const unsigned int& tileW, const unsigned int& tileH, const std::string& name,
                          const MAP_DEPTH& depth, const GIDManager* g, const L_TILED_LAYER_TYPE& type=LAYER_GENERIC);
        virtual ~TiledLayerGeneric(){}

        //Width and Height in Pixels
        const unsigned int pixelWidth;
        const unsigned int pixelHeight;

        //Width and Height in Tiles
        const unsigned int tileWidth;
        const unsigned int tileHeight;

        const L_TILED_LAYER_TYPE layerType;
        const std::string layerName;

        //GID Manager for the RSC_Map that this layer is a part of
        const GIDManager*  GIDM;

        float       GetAlpha  () const {return layerOpacity;}
        int         GetFlags  () const {return layerFlags;  }
        bool        IsVisible () const {return layerVisible;}
        bool        Ignore    () const;
        MAP_DEPTH   GetDepth  () const {return layerDepth;  }

        std::string GetPropertyValue(const std::string& propertyName) const;
        bool        PropertyExists  (const std::string& propertyName) const;

    protected:
        int         layerFlags;
        MAP_DEPTH   layerDepth;
        float       layerOpacity;
        bool        layerVisible;

        PropertyMap properties;
};

class TiledTileLayer : public TiledLayerGeneric{
    friend RSC_MapImpl;
    friend TiledData;

    public:
        TiledTileLayer(const unsigned int& w, const unsigned int& h, const std::string& name, const MAP_DEPTH& depth, const GIDManager* g);
        TiledTileLayer(const TiledTileLayer& rhs, const GIDManager* g);

		/**
		 * Gets GID at Tile-Coordinate x,y
		 */
        GID GetGID(unsigned int x, unsigned int y) const;
		/**
		 * Sets GID at Tile-Coordinate x,y
		 */
        void SetGID(unsigned int x, unsigned int y, GID id);
		/**
		 * Checks if GID at Tile-Coordinate x,y is not 0
		 */
        bool HasTile(unsigned int x, unsigned int y) const;

		///Calls 'GetTileProperty' from tiledset
		std::string GetTileProperty(unsigned int x, unsigned int y, const std::string& property);

        //LuaInterface Functions
        unsigned int GetTile(unsigned int x, unsigned int y) const;
        void SetTile(unsigned int x, unsigned int y, unsigned int id);

        //Lua Functions
        bool    IsSolid         () const;
        bool    IsDestructible  () const;
        bool    UsesHMaps       () const;
        float   GetFriction     () const;
        //All set tiles will be updated on the GPU
        void    UpdateRenderArea(CRect area);

        unsigned int GetAnimationRate () const {return animatedRefreshRate;  }

        TiledSet* GetTiledSet() const {return tileSet;}

        mutable std::vector<CRect> updatedAreas;

    protected:
        //Each tiled Layer is allowed one tiled set
		TiledSet* tileSet;

        float   friction;
        unsigned int animatedRefreshRate;

    private:
        std::vector<std::vector<GID> > data2D;

        void InitializeMap();
		bool inline IsValidXY(unsigned int x, unsigned int y) const;
};

class TiledImageLayer : public TiledLayerGeneric{
    public:
        TiledImageLayer(const unsigned int& pixelW, const unsigned int& pixelH, const std::string& name, const MAP_DEPTH& depth, const GIDManager* g, const RSC_Texture* tex);
        TiledImageLayer(const TiledImageLayer& rhs, const GIDManager* g);

        void SetTexture     (const std::string& textureName);
        void SetOffset      (const Coord2d& off);
        void SetParalax     (const Coord2d& para);

        const RSC_Texture* GetTexture() const{return texture;}

    private:
        Coord2d     offset;
        Coord2df    paralax;
        const RSC_Texture*   texture;
};

//A Tiled Object is simply a bag of data that is used to instantiate an entity when the map is loaded
class TiledObject{
    public:
        std::string name, type, script;
        int x;
        int y;
        int w;
        int h;
        EID parent;
        bool useEntrance;
        bool light;
        void* extraData;

        EID tiledID;

        int flags;

        std::vector<EID> eventSources;

        std::map<std::string, int        > intProperties;
        std::map<std::string, bool       > boolProperties;
        std::map<std::string, float      > floatProperties;
        std::map<std::string, std::string> stringProperties;
};

class TiledObjectLayer : public TiledLayerGeneric{
    public:
        TiledObjectLayer(const unsigned int& pixelW, const unsigned int& pixelH, const std::string& name, const MAP_DEPTH& depth, const GIDManager* g);
        TiledObjectLayer(const TiledObjectLayer& rhs, const GIDManager* g);

        std::map<EID,TiledObject> objects;
};

struct MapEntrance{
    unsigned int mEntranceID;
    CRect mPosition;
};

struct MapExit{
    unsigned int mEntranceID;
    CRect mPosition;
    std::string mMapName;
};

/*TERMINOLOGY
A Map Entrance is a ID with a position
An Exit is an Entrance with a map name string (even if the string is blank; referring to the previous map)
When you collide with an exit, you go to a new map
You don't collide with entrances
An entrance can only be used to enter the map
An Exit can only be used to exit the map
*/

/**
 * Encapsulates data from Tiled, TiledData owns sets, layers, and objects
 */
class TiledData{
    typedef std::map<int, MapEntrance> tEntrances;
    typedef std::map<int, MapExit> tExits; //leave string blank ("") to refer to previous map

    //For the purposes of testing collision; introduces coupling, may want to find a better solution
    friend ComponentCollisionManager;
    friend StateManager;
    friend GameState;
    friend RSC_MapImpl;

    public:
         TiledData(const unsigned int& tWidth, const unsigned int& tHeight);
         TiledData(const TiledData& rhs);
        ~TiledData();

        //returns false if the depth is already taken
        bool AddLayer   (std::unique_ptr<TiledLayerGeneric> layer);
        bool AddTileSet (std::unique_ptr<TiledSet> tileSet       );

        unsigned int GetWidth (){return width; }
        unsigned int GetHeight(){return height;}

        GIDManager gid;
        const unsigned int width;
        const unsigned int height;
        const unsigned int tileWidth;
        const unsigned int tileHeight;
        int bgColor;

        //this data structure enables layers to be looked up by their name, this structure does not assume ownership of the layers
        std::map<std::string, TiledLayerGeneric*> tiledLayers;
        //this data structure enables sets to be looked up by their name, this structure does not assume ownership of the sets
        std::map<std::string, TiledSet*> tiledSets;
        //contains tile layers and image layers sorted by map depth, this structure does not assume ownership of the layers
        std::map <MAP_DEPTH, TiledLayerGeneric*>   tiledRenderableLayers;


        static std::unique_ptr<TiledData> LoadResourceFromTMX(const std::string& TMXname, const char* dat, unsigned int fsize);


        std::string GetProperty (const std::string& property);
        //Can access tiled layers by their name
        TiledTileLayer* GetTileLayer(const std::string& name);

    protected:
        //These variable names are brought to you by the redundancy department of redundancy
        std::vector<std::unique_ptr<TiledSet> >                   tiledTileSets;
        std::vector<std::unique_ptr<TiledObjectLayer> >           tiledObjectLayers;
        std::vector<std::unique_ptr<TiledImageLayer> >            tiledImageLayers;
        std::vector<std::unique_ptr<TiledTileLayer> >             tiledTileLayers;

        tExits mMapExits;
        tEntrances mMapEntrances;

    private:
        //Give the function a string, its type, and where to store the data
        static void TMXProcessType(std::string& type, std::string& value, void* data);

        //for use with an empty property map, stores all properties found with type information
        static void TMXLoadProperties(rapidxml::xml_node<>* rootPropertyNode, PropertyMap& properties);

        //Will insert data from the property map into the data pointed by the attribute map
        static void TMXLoadAttributesFromProperties(const PropertyMap* properties, AttributeMap& attributes);

        //For use with either an empty or populated attribute map
        static void TMXLoadAttributes(rapidxml::xml_node<>* rootAttributeNode, AttributeMap& attributes);
        static void TMXProcessEventListeners(std::string& listenersString, std::vector<EID>& listeners);

        static std::unique_ptr<TiledSet>           TMXLoadTiledSet         (rapidxml::xml_node<>* tiledSetRootNode,  const GID& firstGID, GIDManager& gidManager);
        static std::unique_ptr<TiledTileLayer>     TMXLoadTiledTileLayer   (rapidxml::xml_node<>* rootNode, const GIDManager& gidManager    );
        static std::unique_ptr<TiledObjectLayer>   TMXLoadTiledObjectLayer (rapidxml::xml_node<>* rootNode, TiledData* tiledData    );
        static std::unique_ptr<TiledImageLayer>    TMXLoadTiledImageLayer  (rapidxml::xml_node<>* rootNode, const GIDManager& gidManager    );

        PropertyMap properties;
};


class ComponentScript;

/**
 * Interface for the Resource "Map"
 * Encapsulates a map used by the engine
 */
class RSC_Map{
    public:
        class Exception : public LEngineException{using LEngineException::LEngineException;};

        RSC_Map();
        virtual ~RSC_Map();
        virtual int GetWidthTiles() const   = 0;
        virtual int GetHeightTiles() const  = 0;
        virtual int GetWidthPixels() const  = 0;
        virtual int GetHeightPixels() const = 0;

        virtual std::string GetProperty (const std::string& property) const = 0;
        virtual std::string GetMapName  () const = 0;
        virtual TiledTileLayer* GetTileLayer(const std::string& name) = 0;
        //returns 0 if name doesn't exist
        virtual EID GetEIDFromName(const std::string& name) const = 0;

        virtual const TiledTileLayer* GetTileLayerCollision(int x,  int y, bool areTheseTileCoords) const = 0;

		/// \TODO remove 'GetTiledData' from RSC_MAP Interface, the user of this class should not need to access the Tiled Data
        virtual TiledData* GetTiledData() = 0;
};

class RSC_MapImpl : public RSC_Map{;;
    friend GameState;

    public:
        RSC_MapImpl(std::unique_ptr<TiledData> td);
        RSC_MapImpl(const RSC_MapImpl& rhs);
        ~RSC_MapImpl();

        //Data
        std::unique_ptr<TiledData> tiledData;

        static std::unique_ptr<RSC_Map> LoadResource(const std::string& fname);

        int GetWidthTiles() const;
        int GetHeightTiles() const;
        int GetWidthPixels() const;
        int GetHeightPixels() const;

        std::string GetProperty (const std::string& property) const;
        std::string GetMapName  () const;
        TiledTileLayer* GetTileLayer(const std::string& name);
        EID GetEIDFromName(const std::string& name) const;

        const TiledTileLayer* GetTileLayerCollision(int x, int y, bool areTheseTileCoords) const;

        TiledData* GetTiledData();

    private:
        std::string mMapName;

        //!Every child's GID is incremented by the first (lowest) gid allowed for the map 
        GID firstGID;
};

#endif
