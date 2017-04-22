#ifndef L_TILEMAP
#define L_TILEMAP

#include "Resources/LMap.h"
#include "Resources/LTDF.h"
#include "Resources/LSprite.h"
#include "Resources/LBackground.h"
#include "Resources/LEvent.h"
#include "Errorlog.h"
#include "RenderManager.h"

#include <vector>
#include <map>
#include <string>
#include <list>

//Tile Map owns Backgrounds
//Tile Map has it's own copy of LTDFs

class MapBackground : public RenderableObject{
    public:
        MapBackground(std::string bg, MAP_DEPTH depth);
        ~MapBackground();

        LSprite* sprite;
        LBackground* background;

        std::string bgstr, sprstr, anistr;

        void ReloadBG();
        void ReloadSprite();

        int oldCamX;
        int oldCamY;
        int offsetX;
        int offsetY;

        void Render();
};

class MapEvent{
    public:
        MapEvent(std::string eve, int xx, int yy, int ww, int hh, MAP_DEPTH d);
        MapEvent(std::string eve, CRect coord, MAP_DEPTH d);

        int depth;

        void ReloadEvent();

        std::string eventName;
        CRect coords;
        LEvent* event;
};

class TileMap{
    public:
        ~TileMap();

        //Map each depth value to a 2D tilemap
        typedef std::vector< std::vector< LTDF* > >  TMAP2D;
        typedef std::map<MAP_DEPTH, TMAP2D> TMAP3D;

        typedef std::map<MAP_DEPTH, MapBackground*> BackgroundMap;
        typedef std::list<MAP_DEPTH> DEPTHLIST;

        typedef std::list<MapEvent*>  EventList;
        typedef EventList::iterator EventListIt;

        //Map of all LTDFs
        typedef std::map<std::string, LTDF*> DefMap;
        typedef DefMap::iterator DefIt;

        void Startup(unsigned int w, unsigned int h);

        bool SetTile(std::string name, unsigned int x, unsigned int y, MAP_DEPTH depth=0);
        inline LTDF* GetTile(unsigned int x, unsigned int y, MAP_DEPTH depth=0){
            if((x>=width) or (y>=height)){return NULL;}
            return tileMap[depth][x][y];
        }

        bool AddTileDef(std::string name, std::string spr, std::string ani, std::string hmap, double friction, int32_t flags);
        bool AddTileDef(std::string name);
        LTDF* GetTileDef(std::string name);
        void DeleteTileDef(std::string name);

        int BGGetSize(){return BGs.size();}
        bool AddBackground(MapBackground* bg);
        MapBackground* GetBackground(MAP_DEPTH d);
        void DeleteBackground(MAP_DEPTH);

        int EventGetSize(){return events.size();}
        bool AddEvent(MapEvent* event);
        void DeleteEvent(EventListIt i);

        EventListIt GetEventsBegin(){return events.begin();}
        EventListIt GetEventsEnd(){return events.end();}

        BackgroundMap::iterator GetBackgroundsBegin(){return BGs.begin();}
        BackgroundMap::iterator GetBackgroundsEnd(){return BGs.end();}

        inline unsigned int GetWidth(){return width;}
        inline unsigned int GetHeight(){return height;}

        void InitTileLayer(MAP_DEPTH depth);
        void ClearMaps();
        void ClearDefs();
        void ClearBackgrounds();
        void ClearEvents();

        void CreateFromTiledMap(LMap* lm);

    private:
        unsigned int width, height;

        DefMap defs;
        TMAP3D tileMap;
        DEPTHLIST registeredDepths;
        BackgroundMap BGs;
        EventList events;

        std::list<RenderTileLayer*> renderPointers;
};

#endif
