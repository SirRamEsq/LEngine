#ifndef L_ENGINE_LUAINTERFACE
#define L_ENGINE_LUAINTERFACE

//Engine headers
#include "Defines.h"

#include "Components/CompPosition.h"
#include "Components/CompSprite.h"
#include "Components/CompCollision.h"
#include "Components/CompParticle.h"

#include "Resources/LScript.h"
#include "Resources/LMap.h"

//Lua headers
extern "C"{
  #include "lua5.2/lua.h"
  #include "lua5.2/lualib.h"
  #include "lua5.2/lauxlib.h"
}
#include <LuaBridge.h>


//Forward declares
class GameState;
class ComponentPosition;
class ComponentCollision;
class ComponentParticle;

class LuaInterface{
    friend GameState;
    public:
        //Initializes and exposes the C++ functionality to the luaState
        LuaInterface(GameState* state, const int& resX, const int& resY, const int& viewX, const int& viewY);
        ~LuaInterface();

        bool RunScript(EID id, const LScript* script, MAP_DEPTH depth, EID parent, const std::string& name, const std::string& type,
           /*optional args*/    const TiledObject* obj, luabridge::LuaRef* initTable);

        ///////////
        //General//
        ///////////
            void ListenForInput (EID id, const std::string& inputName );
            void WriteError     (EID id, const std::string& error     );
            //sends event to audio subsystem
            void PlaySound      (const std::string& sndName);
            const LSprite* LoadSprite(const std::string& sprPath);

        //////////////
        //Components//
        //////////////
            //bool create determines whether or not to create a component if it doesn't exist
            ComponentPosition*  GetPositionComponent (const EID& id);
            ComponentSprite*    GetSpriteComponent   (const EID& id);
            ComponentCollision* GetCollisionComponent(const EID& id);
            ComponentParticle*  GetParticleComponent (const EID& id);
            ComponentCamera*    GetCameraComponent   (const EID& id);

        ////////////
        //Entities//
        ////////////
            EID               EntityGetInterfaceByName  (const std::string& name);
            luabridge::LuaRef EntityGetInterface        (const EID& id);
            Coord2df          EntityGetPositionWorld    (EID entity);
            Coord2d           EntityGetPositionWorldInt (EID entity);
            Coord2df          EntityGetMovement         (EID entity);

            EID EntityNew    (const std::string& scriptName, int x, int y, MAP_DEPTH depth, EID parent,
                              const std::string& name, const std::string& type,
                              luabridge::LuaRef propertyTable);
            void EntityDelete(EID entity);

        /////////////
        //Rendering//
        /////////////
            RenderText* RenderObjectText    (EID selfID, int x, int y, const std::string& text, bool abss=true);
            RenderLine* RenderObjectLine    (EID selfID, int x, int y, int xx, int yy);
            void        RenderObjectDelete  (EID selfID, RenderableObject* obj);

        //////////
        //Events//
        //////////
            //These functions send out events to lua scripts
            //Listen to all events from a certain entity
            void EventLuaObserveEntity  (EID listenerID, EID senderID);
            void EventLuaBroadcastEvent (EID senderID, const std::string& event);
            void EventLuaSendEvent      (EID senderID, EID recieverID, const std::string& event);

        ///////////
        //Handles//
        ///////////
            LMap* GetMap();


        static const std::string LUA_52_INTERFACE_ENV_TABLE;
        static const std::string DEBUG_LOG;
        static const std::string BASE_SCRIPT_NAME;
        static const std::string TYPE_DIR;

        const int& RESOLUTION_X;
        const int& RESOLUTION_Y;
        const int& VIEWPORT_X;
        const int& VIEWPORT_Y;

    protected:
        lua_State* GetState() {return lState;}

    private:
        //All entities derive from this
        const LScript* baseScript;
        int baseLuaClass;

        int LoadScriptFromChunk(const LScript* script);

        //Creates 'CPP' table in the global table
        void ExposeCPP();

        //used to access component managers
        GameState* parentState;

        //this class is the owner of the lua state for the current gamestate
        lua_State* lState;

        //Map of scripts to (a reference of) their lua functions that return a new instance.
        std::map<const LScript*, int> classes;
};

#endif //L_ENGINE_LUAINTERFACE
