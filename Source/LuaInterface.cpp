#include "LuaInterface.h"
#include "Exceptions.h"
#include "StateManager.h"
#include "Kernel.h"

#include <sstream>

void stackdump_g(lua_State* l, const std::string& logFile)
{
    ErrorLog::WriteToFile("===", logFile);  /* put a separator */
    ErrorLog::WriteToFile("START STACK DUMP", logFile);
    int i;
    int top = lua_gettop(l);

    std::stringstream ss;
    ss << "total in stack " << top;
    ErrorLog::WriteToFile(ss.str(), logFile);

    std::stringstream s;

    for (i = 1; i <= top; i++)
    {  /* repeat for each level */
        int t = lua_type(l, i);
        s << "---\n"; /* put a separator */
        switch (t) {
            case LUA_TSTRING:  /* strings */
                s << "string: " << lua_tostring(l, i) << "\n";
                break;
            case LUA_TBOOLEAN:  /* booleans */
                s << "boolean: " << (lua_toboolean(l, i) ? "true" : "false") << "\n";
                break;
            case LUA_TNUMBER:  /* numbers */
                s << "number: " << lua_tonumber(l, i) << "\n";
                break;
            default:  /* other values */
                s << "type is: " << lua_typename(l, t) << "\n";
                break;
        }
    }
    ErrorLog::WriteToFile(s.str(), logFile);
    ErrorLog::WriteToFile("===", logFile);  /* put a separator */
}

int GetErrorInfo(lua_State* L){
   lua_Debug d;
   lua_getstack(L, 1, &d);
   lua_getinfo(L, "Sln", &d);
   std::string err = lua_tostring(L, -1);
   lua_pop(L, 1);
   std::stringstream msg;
   msg << d.short_src << ":" << d.currentline;

   if (d.name != 0)
   {
      msg << "(" << d.namewhat << " " << d.name << ")";
   }
   msg << " " << err;

   ErrorLog::WriteToFile("Lua Error: ", msg.str());
   lua_pushstring(L, msg.str().c_str());
   return 1;
}

using namespace luabridge;


//Base Entity needs to be inside Type_Dir
const std::string LuaInterface::BASE_SCRIPT_NAME = "__BaseEntity.lua";
const std::string LuaInterface::TYPE_DIR = "Types/";

const std::string LuaInterface::DEBUG_LOG="LUA_INTERFACE";
const std::string LuaInterface::LUA_52_INTERFACE_ENV_TABLE=
//http://stackoverflow.com/questions/34388285/creating-a-secure-lua-sandbox

//Wrapper around loadfile; Files can only load new files if they specify an _ENV
"newLoadFile = function(dir, env)                                          \n"
	"if (env==nil) then return nil; end                                    \n"
	"return loadfile(dir, \"bt\", env)                                     \n"
"end                                                                       \n"
//Global table for this environment
//Note, that in lua 5.2, there is nothing special about _G;
//_ENV is the global table for a given function; _G is simply set at the start of the entire lua state
"L_ENGINE_ENV_G = {}                                                       \n"
"L_ENGINE_ENV = {                                                          \n"

    //Global table between all scripts
    "_G = L_ENGINE_ENV_G,                                                       \n"
    //It is expected to have exposed CPP data (thus creating the CPP table) before running this string
    "CPP = CPP,                                                                 \n"
    "commonPath= commonPath,                                                    \n"
    //Use secure wrapper around loadfile
    "loadfile = newLoadFile,                                                    \n"
    //Built in Lua functions
    "ipairs = ipairs,                                                           \n"
    "next = next,                                                               \n"
    "pairs = pairs,                                                             \n"
    "pcall = pcall,                                                             \n"
    "tonumber = tonumber,                                                       \n"
    "tostring = tostring,                                                       \n"
    "type = type,                                                               \n"
    //Can set metatables, but not get
    "setmetatable = setmetatable,                                               \n"
    "unpack = unpack,                                                           \n"
    "coroutine = {                                                              \n"
        "create = coroutine.create, resume = coroutine.resume,                  \n"
        "running = coroutine.running, status = coroutine.status,                \n"
        "wrap = coroutine.wrap                                                  \n"
    "},                                                                         \n"
    "string = {                                                                 \n"
        "byte = string.byte, char = string.char, find = string.find,                \n"
        "format = string.format, gmatch = string.gmatch, gsub = string.gsub,        \n"
        "len = string.len, lower = string.lower, match = string.match,              \n"
        "rep = string.rep, reverse = string.reverse, sub = string.sub,              \n"
        "upper = string.upper                                                       \n"
    "},                                                                         \n"
    "table = {                                                                  \n"
        "insert = table.insert, maxn = table.maxn, remove = table.remove,           \n"
        "sort = table.sort                                                          \n"
    "},                                                                         \n"
    "math = {                                                                   \n"
        "abs = math.abs, acos = math.acos, asin = math.asin,                        \n"
        "atan = math.atan, atan2 = math.atan2, ceil = math.ceil, cos = math.cos,    \n"
        "cosh = math.cosh, deg = math.deg, exp = math.exp, floor = math.floor,      \n"
        "fmod = math.fmod, frexp = math.frexp, huge = math.huge,                    \n"
        "ldexp = math.ldexp, log = math.log, log10 = math.log10, max = math.max,    \n"
        "min = math.min, modf = math.modf, pi = math.pi, pow = math.pow,            \n"
        "rad = math.rad, random = math.random, sin = math.sin, sinh = math.sinh,    \n"
        "sqrt = math.sqrt, tan = math.tan, tanh = math.tanh                         \n"
    "},                                                                         \n"
    "os = { clock = os.clock, difftime = os.difftime, time = os.time },         \n"
"}                                                                          \n";

LuaInterface::LuaInterface(GameState* state, const int& resX, const int& resY, const int& viewX, const int& viewY)
                        : parentState(state), RESOLUTION_X(resX), RESOLUTION_Y(resY), VIEWPORT_X(viewX), VIEWPORT_Y(viewY){
    lState = luaL_newstate();

    if(lState==NULL){
        std::stringstream ss;
        ss << "Lua state couldn't be created";
        ErrorLog::WriteToFile(ss.str(), DEBUG_LOG);
        return;
    }

    luaL_openlibs(lState);
    //Create 'CPP' table referenced by LUA_52_INTERFACE_ENV_TABLE
    ExposeCPP();

    /*
    Could probably remove the dependency on LuaFileSystem entirely if I just pass the current directory from the engine
    */
    //Set Lua common directory using luaFileSystem
    std::string lfsLoad1    = "lfs = require(\"lfs\")";

    //Append to package.path
    //std::string lfsLoad2    = "package.path = package.path .. \";\" .. lfs.currentdir() .. \"/Data/Resources/Scripts/Common/?.lua\" ";
    //std::string lfsLoad3    = "LEngineInitPath = lfs.currentdir() .. \"/Data/Resources/Scripts/Common/LEngineInit.lua\" ";

    std::string lfsLoad2    = "commonPath       = lfs.currentdir() .. \"/Data/Resources/Scripts/Common\" ";
    std::string lfsLoad3    = "LEngineInitPath  = commonPath .. \"/LEngineInit.lua\" ";

    //run InitLEngine script using the restricted environment
    std::string lEngineLoad1 = "f1 = loadfile(LEngineInitPath, \"bt\", L_ENGINE_ENV)";
    //Run the loadfile, returning a function
    std::string lEngineLoad2 = "NewLEngine = f1()";

    const std::string* doStrings[6];
    doStrings[0] = &lfsLoad1;
    doStrings[1] = &lfsLoad2;
    doStrings[2] = &lfsLoad3;
    doStrings[3] = &LUA_52_INTERFACE_ENV_TABLE;
    doStrings[4] = &lEngineLoad1;
    doStrings[5] = &lEngineLoad2;

    int error=0;
    for(int i=0; i<=5; i++){
        error = luaL_dostring(lState, doStrings[i]->c_str());
        if(error!=0){
            std::stringstream ss;
            ss << "Lua String could not be run " << *doStrings[i]
            << " | Error code is: " << error << " "
            << "   ...Error Message is " << lua_tostring(lState,-1);
            ErrorLog::WriteToFile(ss.str(), DEBUG_LOG);
            lua_pop(lState, -1); //pop error message
            //GetErrorInfo(lState);
            return;
        }
    }

    LuaRef engineFunction = getGlobal(lState, "NewLEngine");
    if (engineFunction.isNil()) {
        ErrorLog::WriteToFile("NewLEngine is nil", DEBUG_LOG);
    }
    if (!engineFunction.isFunction()) {
        ErrorLog::WriteToFile("NewLEngine is not function", DEBUG_LOG);
    }

    //GetBaseEntity Creator
    std::string scriptName = TYPE_DIR + BASE_SCRIPT_NAME;
    baseScript = K_ScriptMan.GetItem(scriptName);
    if(baseScript==NULL){
        K_ScriptMan.LoadItem(scriptName,scriptName);
        baseScript = K_ScriptMan.GetItem(scriptName);
        if(baseScript==NULL){
            ErrorLog::WriteToFile("Cannot load base entity lua script", DEBUG_LOG);
            ErrorLog::WriteToFile(scriptName, DEBUG_LOG);
        }
    }
    if(baseScript!=NULL){
        try{
            int functionReference = LoadScriptFromChunk(baseScript);

            //saves function for later use
            baseLuaClass = functionReference;
        }
        catch(LEngineException e){
            ErrorLog::WriteToFile(e.what(), DEBUG_LOG);
        }
    }

    // /Data/Resources/Scripts/Common can now be accessed by all scripts (though require isn't currently allowed)
    // lfs is nil
    // NewLEngine is now a function that can be used to created new instances of LEngine
    // classes[baseScript] can be used to get a new base entity
}

LuaInterface::~LuaInterface(){

}

int LuaInterface::LoadScriptFromChunk(const LScript* script){
    //////////////
    //Load Chunk//
    //////////////
    int error = luaL_loadbufferx(lState, script->script.c_str(), script->script.length(), script->scriptName.c_str(), "bt");
    if(error !=0){
        std::stringstream errorMsg;
        errorMsg << "Script [" << script->scriptName << "] could not be loaded from C++\n"
        << "   ...Error code "  << std::to_string(error) << "\n"
        << "   ...Error Message is " << lua_tostring(lState,-1);
        // completely clear the stack before return
        lua_settop(lState, 0);
        throw LEngineException(errorMsg.str());
    }

    lua_getglobal(lState, "L_ENGINE_ENV");   //push environment onto stack
    lua_setupvalue(lState, -2, 1);           //pop environment and assign to upvalue#1 (the function environment)

    if(int error= lua_pcall (lState, 0, 1, 0)  != 0){//Calling pcall pops the function and its args [#args is arg 1] from the stack
                                                        //The result [# of returns is arg 2] is pushed on the stack
                                                        //should be a function that can create
                                                        //a table with "Update", "Init", etc... methods
        std::stringstream errorMsg;
        errorMsg << "Script [" << script->scriptName << "] could not be run from C++\n"
        << "   ...Error code "  << std::to_string(error) << "\n"
        << "   ...Error Message is " << lua_tostring(lState,-1);
        // completely clear the stack before return
        lua_settop(lState, 0);
        throw LEngineException(errorMsg.str());
    }

    //Get top of stack (returned value from pcall)
    int stackTop = lua_gettop(lState);
    if(lua_isfunction(lState, stackTop)==false){ //Returned value isn't table; cannot be used
        std::stringstream errorMsg;
        errorMsg << "Returned value is not function in script [" << script->scriptName
        << "] \n"
        << " ...Type is: [" << lua_typename(lState, lua_type(lState, stackTop)) << "]";
        // completely clear the stack before return
        lua_settop(lState, 0);
        throw LEngineException(errorMsg.str());
    }

    //pops function off of stack
    return luaL_ref(lState, LUA_REGISTRYINDEX);
}

///////////
//General//
///////////
//Clears stack
bool LuaInterface::RunScript(EID id, const LScript* script, MAP_DEPTH depth, EID parent, const std::string& name, const std::string& type,
                             const TiledObject* obj, LuaRef* initTable){

    /*
    Returned value from script should be creator function that can create multiple instances of the entity it describes
    Value is then saved and if another instance of this script is called, the returned lua function is simply called
    to create another instance
    */

    //reference to function to be called to generate table containing class instance
    int functionReference = 0;

    //check if entity has script component

    ComponentScript* scriptComponent = (ComponentScript*) parentState->comScriptMan.GetComponent(id);
    if(scriptComponent == NULL){
        std::stringstream ss;
        ss << "Couldn't run script for entity with EID " << id << " as this entity does not have a script component";
        throw LEngineException (ss.str());
    }

    ////////////////////////////////////////
    //Check if script has already been run//
    ////////////////////////////////////////

    auto classDefinition = classes.find(script);
    if(classDefinition!=classes.end()){
        functionReference = classDefinition->second;
    }

    else{
        //Load chunk if it hasn't been loaded before
        try{
            functionReference = LoadScriptFromChunk(script);
        }
        catch(LEngineException e){
            ErrorLog::WriteToFile(e.what(), DEBUG_LOG);
            return false;
        }

        //saves function for later use
        classes[script] = functionReference;
    }

    //pushes class factory function back on the stack
    lua_rawgeti(lState, LUA_REGISTRYINDEX, functionReference);

    if(baseScript != NULL){
        //push baseClass and calls function
        lua_rawgeti(lState, LUA_REGISTRYINDEX, baseLuaClass);
        //Call function and place table at the top of the stack
        if(int error= lua_pcall (lState, 0, 1, 0)  != 0){
            std::stringstream ss;
            ss << "Base class did not return a callable function\n" << "   ...Error code "  << std::to_string(error) << "\n";
            // completely clear the stack before return
            lua_settop(lState, 0);
            ErrorLog::WriteToFile(ss.str(), DEBUG_LOG);
            return false;
        }
    }
    else{
        lua_pushnil(lState);
    }

    //Call function (passing either baseclass or nil) and place table at the top of the stack
    if(int error= lua_pcall (lState, 1, 1, 0)  != 0){
        std::stringstream ss;
        ss << "Script [" << script->scriptName << "] with EID [" << std::to_string(id) << "] did not return a callable function \n"
        << "   ...Error code "  << std::to_string(error) << "\n"
        << "   ...Error Message is " << lua_tostring(lState,-1);
        // completely clear the stack before return
        lua_settop(lState, 0);
        ErrorLog::WriteToFile(ss.str(), DEBUG_LOG);
        return false;
    }

    int stackTop = lua_gettop(lState);
    if(lua_istable(lState, stackTop)==false){ //Returned value isn't table; cannot be used
        std::stringstream errorMsg;
        errorMsg << "Returned value from lua function is not a table in script [" << script->scriptName
        << "] with EID [" << std::to_string(id) << "] \n"
        << " ...Type is: [" << lua_typename(lState, lua_type(lState, stackTop)) << "]";
        ErrorLog::WriteToFile(errorMsg.str(), DEBUG_LOG);
        // completely clear the stack before return
        lua_settop(lState, 0);
        return false;
    }

    //Set 'name+eid" to the returned table containing the 'Update', 'Init', etc... functions
    std::stringstream returnedTableName;
    returnedTableName << name << id;
    lua_setglobal(lState, returnedTableName.str().c_str()); //assign and pop value

    ////////////////////////////////////////////////////////////
    //Assign Instance of LEngine to script for starting values//
    ////////////////////////////////////////////////////////////
    try{
        //Create new Instance
        LuaRef engineRef=getGlobal(lState,"NewLEngine");
        LuaRef engineTableRef = engineRef();
        engineTableRef["Initialize"](id, name, type, this, depth, parent);

            engineTableRef["RESOLUTION_X"]  = RESOLUTION_X;
            engineTableRef["RESOLUTION_Y"]  = RESOLUTION_Y;
            engineTableRef["VIEWPORT_X"]    = VIEWPORT_X;
            engineTableRef["VIEWPORT_Y"]    = VIEWPORT_Y;

        //Assign Instance to generated table
        LuaRef returnedTable=getGlobal(lState, returnedTableName.str().c_str());
        returnedTable["LEngineData"]= engineTableRef;
    }
    catch(std::exception e){
        ErrorLog::WriteToFile("Couldn't Run LEngineInit; ");
        ErrorLog::WriteToFile(e.what());
    }

    //////////////////////////////////////////////////
    //Assign values from optional args to the script//
    //////////////////////////////////////////////////

    if(obj!=NULL){
        LuaRef returnedTable=getGlobal(lState, returnedTableName.str().c_str());
        LuaRef lengineData = returnedTable["LEngineData"];
        LuaRef initTable = lengineData["InitializationTable"];
        for (auto i = obj->boolProperties.begin(); i != obj->boolProperties.end(); i++){
            initTable[i->first] = i->second;
        }
        for (auto i = obj->intProperties.begin(); i != obj->intProperties.end(); i++){
            initTable[i->first] = i->second;
        }
        for (auto i = obj->floatProperties.begin(); i != obj->floatProperties.end(); i++){
            initTable[i->first] = i->second;
        }
        for (auto i = obj->stringProperties.begin(); i != obj->stringProperties.end(); i++){
            initTable[i->first] = i->second;
        }
    }

    if(initTable!=NULL){
        LuaRef returnedTable=getGlobal(lState, returnedTableName.str().c_str());
        LuaRef lengineData = returnedTable["LEngineData"];
        lengineData["InitializationTable"] = (*initTable);
    }

    //////////////////////////////////////////////////////////////////////////////////////
    //Create new script component and assign reference to the new table to the component//
    //////////////////////////////////////////////////////////////////////////////////////
    LuaRef returnedTable=getGlobal(lState, returnedTableName.str().c_str());

    //Initialize Component which has already been created
    scriptComponent->SetScriptPointerOnce(returnedTable);
    scriptComponent->scriptName=script->scriptName;
    scriptComponent->RunFunction("Initialize");

    return true;
}

const LSprite* LuaInterface::LoadSprite(const std::string& sprPath){
    const LSprite* sprite=K_SpriteMan.GetItem(sprPath);
    if(sprite==NULL){
        if(K_SpriteMan.LoadItem(sprPath,sprPath)==false){
            ErrorLog::WriteToFile("LuaInterface::LoadSprite; Couldn't Load Sprite Named: " + sprPath, DEBUG_LOG);
            return NULL;
        }
        sprite=K_SpriteMan.GetItem(sprPath);
    }
    return sprite;
}

void LuaInterface::ListenForInput (EID id, const std::string& inputName){
     K_InputMan.ListenForInput(inputName, id);
}

void LuaInterface::WriteError     (EID id, const std::string& error){
    const std::string& name=((ComponentScript*)parentState->comScriptMan.GetComponent(id))->scriptName;
    std::stringstream ss;
    ss << "[ LUA" << " | " << name << " | EID: " << id << " ]   " << error;

    ErrorLog::WriteToFile(ss.str(), DEBUG_LOG);
}

void LuaInterface::PlaySound      (const std::string& sndName                     ){
    SoundEvent snd(sndName, 100);
    if(snd.sound!=NULL){
        K_AudioSub.PushSoundEvent(snd);
    }
}

//////////////
//Components//
//////////////
ComponentPosition*  LuaInterface::GetPositionComponent (const EID& id){
    if(parentState->comPosMan.HasComponent(id)==false){
        //if(create){
            parentState->comPosMan.AddComponent(id);
        //}
        //else{return NULL;}
    }
    return (ComponentPosition*)(parentState->comPosMan.GetComponent(id));
}
ComponentSprite*    LuaInterface::GetSpriteComponent   (const EID& id){
    if(parentState->comSpriteMan.HasComponent(id)==false){
        //if(create){
            parentState->comSpriteMan.AddComponent(id);
        //}
        //else{return NULL;}
    }
    return (ComponentSprite*)(parentState->comSpriteMan.GetComponent(id));
}
ComponentCollision* LuaInterface::GetCollisionComponent(const EID& id){
    if(parentState->comCollisionMan.HasComponent(id)==false){
        //if(create){
            parentState->comCollisionMan.AddComponent(id);
        //}
        //else{return NULL;}
    }
    return (ComponentCollision*)(parentState->comCollisionMan.GetComponent(id));
}
ComponentParticle*  LuaInterface::GetParticleComponent (const EID& id){
    if(parentState->comParticleMan.HasComponent(id)==false){
        //if(create){
            parentState->comParticleMan.AddComponent(id);
        //}
        //else{return NULL;}
    }
    return (ComponentParticle*)parentState->comParticleMan.GetComponent(id);
}

ComponentCamera*  LuaInterface::GetCameraComponent (const EID& id){
    if(parentState->comCameraMan.HasComponent(id)==false){
        //if(create){
            parentState->comCameraMan.AddComponent(id);
        //}
        //else{return NULL;}
    }
    return (ComponentCamera*)(parentState->comCameraMan.GetComponent(id));
}

////////////
//Entities//
////////////
EID LuaInterface::EntityGetInterfaceByName(const std::string& name){

}

luabridge::LuaRef LuaInterface::EntityGetInterface (const EID& id){
    return ((ComponentScript*)Kernel::stateMan.GetCurrentState()->comScriptMan.GetComponent(id))->GetEntityInterface();
}

Coord2df LuaInterface::EntityGetPositionWorld(EID entity){
    ComponentPosition* pos = ((ComponentPosition*)Kernel::stateMan.GetCurrentState()->comPosMan.GetComponent(entity));
    if(pos == NULL){
        std::stringstream ss;
        ss << "EntityGetPositionWorld was passed entity id " << entity << " Which does not exist";
        ErrorLog::WriteToFile(ss.str(), DEBUG_LOG);
        return Coord2df(0,0);
    }
    return pos->GetPositionWorld();
}
Coord2d LuaInterface::EntityGetPositionWorldInt(EID entity){
    ComponentPosition* pos = ((ComponentPosition*)Kernel::stateMan.GetCurrentState()->comPosMan.GetComponent(entity));
    if(pos == NULL){
        std::stringstream ss;
        ss << "EntityGetPositionWorldInt was passed entity id " << entity << " Which does not exist";
        ErrorLog::WriteToFile(ss.str(), DEBUG_LOG);
        return Coord2d(0,0);
    }
    return pos->GetPositionWorldInt();
}
Coord2df LuaInterface::EntityGetMovement(EID entity){
    return ((ComponentPosition*)Kernel::stateMan.GetCurrentState()->comPosMan.GetComponent(entity))->GetMovement();
}

EID LuaInterface::EntityNew (const std::string& scriptName, int x, int y, MAP_DEPTH depth, EID parent,
                                const std::string& name, const std::string& type,
                                luabridge::LuaRef propertyTable){
    //New Entity
    EID ent=K_EntMan->NewEntity();

    //New Position Component for Entity
    parentState->comPosMan.AddComponent(ent);
    ((ComponentPosition*)(parentState->comPosMan.GetComponent(ent)) )->SetPositionLocal( Coord2df(x,y) );

    //Get script Data
    const LScript* scriptData=K_ScriptMan.GetItem(scriptName);
    if(scriptData==NULL){
        K_ScriptMan.LoadItem(scriptName,scriptName);
        scriptData=K_ScriptMan.GetItem(scriptName);
        if(scriptData==NULL){
            ErrorLog::WriteToFile("LuaInterface::EntityNew; Couldn't Load Script Named: " + scriptName, DEBUG_LOG);
            return 0;
        }
    }

    //Add script component and run script
    parentState->comScriptMan.AddComponent(ent);

    if(RunScript(ent, scriptData, depth, parent, name, type, NULL, &propertyTable)==false){
        ErrorLog::WriteToFile("Couldn't Create a new Entity", DEBUG_LOG);
        return 0;
    }

    return ent;
}

void LuaInterface::EntityDelete(EID entity){
    K_EntMan->DeleteEntity(entity);
}

/////////////
//Rendering//
/////////////
RenderText* LuaInterface::RenderObjectText    (EID selfID, int x, int y, const std::string& text, bool abss){
    ComponentScript* script=((ComponentScript*)(parentState->comScriptMan.GetComponent(selfID)));
    return script->RenderObjectText(x,y,text,abss);
}
RenderLine* LuaInterface::RenderObjectLine    (EID selfID, int x, int y, int xx, int yy){
    ComponentScript* script=((ComponentScript*)(parentState->comScriptMan.GetComponent(selfID)));
    return script->RenderObjectLine(x,y,xx,yy);
}
void        LuaInterface::RenderObjectDelete  (EID selfID, RenderableObject* obj){
    ComponentScript* script=((ComponentScript*)(parentState->comScriptMan.GetComponent(selfID)));
    script->RenderObjectDelete(obj);
}

//////////
//Events//
//////////
void LuaInterface::EventLuaObserveEntity  (EID listenerID, EID senderID){
    //Get the script that the listener wants to hear
    ComponentScript* senderScript=((ComponentScript*)(parentState->comScriptMan.GetComponent(senderID)));
    ComponentScript* listenerScript=((ComponentScript*)(parentState->comScriptMan.GetComponent(listenerID)));
    if(senderScript==NULL){
        ErrorLog::WriteToFile("Error: In function EventLuaObserveEntity; Cannot find entity with id: " + std::to_string(senderID), DEBUG_LOG);
        return;
    }if(listenerScript==NULL){
        ErrorLog::WriteToFile("Error: In function EventLuaObserveEntity; Cannot find entity with id: " + std::to_string(listenerID), DEBUG_LOG);
        return;
    }

    //Add the listener to the sender's list of observers
    senderScript->EventLuaAddObserver(listenerScript);
}

void LuaInterface::EventLuaBroadcastEvent (EID senderID, const std::string& event){
    ComponentScript* script=((ComponentScript*)(parentState->comScriptMan.GetComponent(senderID)));

    script->EventLuaBroadcastEvent(event);
}

void LuaInterface::EventLuaSendEvent      (EID senderID, EID recieverID, const std::string& event){
    Event e(senderID, recieverID, MSG_LUA_EVENT);
    e.eventDescription=event;

    ComponentScript* script=(ComponentScript*)parentState->comScriptMan.GetComponent(recieverID);
    if(script==NULL){return;}
    script->HandleEvent(&e);
}

LMap* LuaInterface::GetMap(){
    return parentState->GetCurrentMap();
}




void LuaInterface::ExposeCPP(){
    getGlobalNamespace(lState) //global namespace to lua
        .beginNamespace ("CPP") //'CPP' table
            .beginClass<LuaInterface>("LuaInterface") //define class object
                .addFunction("RenderObjectText",    &LuaInterface::RenderObjectText)
                .addFunction("RenderObjectDelete",  &LuaInterface::RenderObjectDelete)
                .addFunction("RenderObjectLine",    &LuaInterface::RenderObjectLine)

                .addFunction("ListenForInput", &LuaInterface::ListenForInput)
                .addFunction("PlaySound", &LuaInterface::PlaySound)

                .addFunction("LoadSprite",  &LuaInterface::LoadSprite)

                .addFunction("WriteError", &LuaInterface::WriteError)

                .addFunction("GetSpriteComponent",      &LuaInterface::GetSpriteComponent)
                .addFunction("GetCollisionComponent",   &LuaInterface::GetCollisionComponent)
                .addFunction("GetPositionComponent",    &LuaInterface::GetPositionComponent)
                .addFunction("GetParticleComponent",    &LuaInterface::GetParticleComponent)

                .addFunction("EntityNew",               &LuaInterface::EntityNew)
                .addFunction("EntityGetInterface",      &LuaInterface::EntityGetInterface)
                .addFunction("EntityDelete",            &LuaInterface::EntityDelete)
                .addFunction("EntityGetInterfaceByName",&LuaInterface::EntityGetInterfaceByName)
                .addFunction("EntityGetPositionWorld",  &LuaInterface::EntityGetPositionWorld)
                .addFunction("EntityGetPositionWorldInt",  &LuaInterface::EntityGetPositionWorldInt)
                .addFunction("EntityGetMovement",       &LuaInterface::EntityGetMovement)

                .addFunction("EventLuaObserveEntity", &LuaInterface::EventLuaObserveEntity)
                .addFunction("EventLuaSendEvent",     &LuaInterface::EventLuaSendEvent)
                .addFunction("EventLuaBroadcastEvent",&LuaInterface::EventLuaBroadcastEvent)

                .addFunction("GetMap",      &LuaInterface::GetMap)

            .endClass()

            .beginClass<TiledTileLayer>("TiledTileLayer") //define class object
                .addFunction("GetFriction",     &TiledTileLayer::GetFriction)
                .addFunction("IsDestructible",  &TiledTileLayer::IsDestructible)
                .addFunction("IsWater",         &TiledTileLayer::IsWater)
                .addFunction("UsesHMaps",       &TiledTileLayer::UsesHMaps)
                .addFunction("HasTile",         &TiledTileLayer::HasTile)
                .addFunction("GetTile",         &TiledTileLayer::GetTile)
                .addFunction("SetTile",         &TiledTileLayer::SetTile)
                .addFunction("UpdateRenderArea",    &TiledTileLayer::UpdateRenderArea)
            .endClass()

            .beginClass<LSprite>("LSprite") //define class object
                .addFunction("GetOrigin", &LSprite::GetOrigin)
                .addFunction("SetOrigin", &LSprite::SetOrigin)
                .addFunction("SetColorKey", &LSprite::SetColorKey)
                .addFunction("GetName", &LSprite::GetName)
            .endClass()

            .beginClass<TColPacket>("TColPacket")
                .addConstructor<void (*)(void)>()
                .addFunction("GetX", &TColPacket::GetX)
                .addFunction("GetY", &TColPacket::GetY)
                .addFunction("GetID", &TColPacket::GetID)
                .addFunction("GetLayer", &TColPacket::GetLayer)
                .addFunction("GetHmap", &TColPacket::GetHmap)
            .endClass()

            .beginClass<EColPacket>("EColPacket")
                .addConstructor<void (*)(void)>()
                .addFunction("GetName", &EColPacket::GetName)
                .addFunction("GetType", &EColPacket::GetType)
                .addFunction("GetID",   &EColPacket::GetID)
            .endClass()

            .beginClass<RenderableObject>("RenderableObject")
                .addFunction("GetRotation", &RenderableObject::GetRotation)
                .addFunction("SetRotation", &RenderableObject::SetRotation)

                .addFunction("GetRender",   &RenderableObject::GetRender)
                .addFunction("SetRender",   &RenderableObject::SetRender)

                .addFunction("SetDepth",    &RenderableObject::SetDepth)

                .addFunction("GetScalingX", &RenderableObject::GetScalingX)
                .addFunction("SetScalingX", &RenderableObject::SetScalingX)

                .addFunction("GetScalingY", &RenderableObject::GetScalingY)
                .addFunction("SetScalingY", &RenderableObject::SetScalingY)
            .endClass()

            .beginClass<ComponentSprite>("ComponentSprite")
                .addFunction("AddSprite",           &ComponentSprite::AddSprite)
                .addFunction("SetAnimation",        &ComponentSprite::SetAnimation)
                .addFunction("SetAnimationSpeed",   &ComponentSprite::SetAnimationSpeed)
                .addFunction("SetImageIndex",       &ComponentSprite::SetImageIndex)

                .addFunction("SetRotation",         &ComponentSprite::SetRotation)
                .addFunction("SetScaling",          &ComponentSprite::SetScaling)
                .addFunction("SetScalingX",         &ComponentSprite::SetScalingX)
                .addFunction("SetScalingY",         &ComponentSprite::SetScalingY)

                .addFunction("SetOffset",           &ComponentSprite::SetOffset)

                .addFunction("RenderSprite",        &ComponentSprite::RenderSprite)
                .addFunction("AnimateSprite",       &ComponentSprite::AnimateSprite)
            .endClass()

            .beginClass<CRect>("CRect")
                .addConstructor<void (*)(int, int, int, int)> ()

                .addData("x", &CRect::x) //Read-Write
                .addData("y", &CRect::y) //Read-Write
                .addData("w", &CRect::w) //Read-Write
                .addData("h", &CRect::h) //Read-Write

                .addFunction("GetTop",      &CRect::GetTop)
                .addFunction("GetBottom",   &CRect::GetBottom)
                .addFunction("GetLeft",     &CRect::GetLeft)
                .addFunction("GetRight",    &CRect::GetRight)
            .endClass()

            .beginClass<ComponentCollision>("ComponentCollision")
                .addFunction("AlwaysCheck",             &ComponentCollision::AlwaysCheck)
                .addFunction("Activate",                &ComponentCollision::Activate)
                .addFunction("Deactivate",              &ComponentCollision::Deactivate)
                .addFunction("SetPrimaryCollisionBox",  &ComponentCollision::SetPrimaryCollisionBox)
                .addFunction("CheckForEntities",        &ComponentCollision::CheckForEntities)
                .addFunction("CheckForTiles",           &ComponentCollision::CheckForTiles)
                .addFunction("ChangeX",                 &ComponentCollision::ChangeX)
                .addFunction("ChangeY",                 &ComponentCollision::ChangeY)
                .addFunction("ChangeWidth",             &ComponentCollision::ChangeWidth)
                .addFunction("ChangeHeight",            &ComponentCollision::ChangeHeight)
                .addFunction("ChangeBox",               &ComponentCollision::ChangeBox)
                .addFunction("AddCollisionBoxInt",      &ComponentCollision::AddCollisionBoxInt)
                .addFunction("AddCollisionBox",         &ComponentCollision::AddCollisionBox)
                .addFunction("SetName",                 &ComponentCollision::SetName)
                .addFunction("SetType",                 &ComponentCollision::SetType)
            .endClass()

            .beginClass<ComponentParticle>("ComponentParticle")
                .addFunction("AddParticleCreator",          &ComponentParticle::AddParticleCreator)
                .addFunction("DeleteParticleCreators",      &ComponentParticle::DeleteParticleCreators)
            .endClass()

            .beginClass<LHeightmap>("LHeightmap")
                .addFunction("GetHeightMapH", &LHeightmap::GetHeightMapH)
                .addFunction("GetHeightMapV", &LHeightmap::GetHeightMapV)

                .addData("angleH", &LHeightmap::angleH)
                .addData("angleV", &LHeightmap::angleV)
            .endClass()


            .beginClass<Coord2d>("Coord2d")
                .addConstructor <void (*) (void)> ()//Empty Constructor
                .addConstructor <void (*) (int, int)> ()//Constructor
                .addData("x", &Coord2d::x)
                .addData("y", &Coord2d::y)
            .endClass()

            .beginClass<Coord2df>("Coord2df")
                .addConstructor <void (*) (void)> ()//Empty Constructor
                .addConstructor <void (*) (float, float)> ()//Constructor
                .addData("x", &Coord2df::x)
                .addData("y", &Coord2df::y)
            .endClass()

            .beginClass<ComponentPosition>("ComponentPosition")
                .addFunction("GetPositionLocal",      &ComponentPosition::GetPositionLocal)
                .addFunction("GetPositionWorld",      &ComponentPosition::GetPositionWorld)
                .addFunction("GetMovement",           &ComponentPosition::GetMovement)
                .addFunction("GetAcceleration",       &ComponentPosition::GetAcceleration)
                .addFunction("GetPositionWorldInt",   &ComponentPosition::GetPositionWorldInt)
                .addFunction("GetPositionLocalInt",   &ComponentPosition::GetPositionLocalInt)

                .addFunction("SetPositionLocal",      &ComponentPosition::SetPositionLocal)
                .addFunction("SetPositionLocalInt",      &ComponentPosition::SetPositionLocalInt)
                .addFunction("SetMovement",           &ComponentPosition::SetMovement)
                .addFunction("SetAcceleration",       &ComponentPosition::SetAcceleration)
                .addFunction("SetPositionLocalX",     &ComponentPosition::SetPositionLocalX)
                .addFunction("SetMovementX",          &ComponentPosition::SetMovementX)
                .addFunction("SetAccelerationX",      &ComponentPosition::SetAccelerationX)
                .addFunction("SetPositionLocalY",     &ComponentPosition::SetPositionLocalY)
                .addFunction("SetMovementY",          &ComponentPosition::SetMovementY)
                .addFunction("SetAccelerationY",      &ComponentPosition::SetAccelerationY)
                .addFunction("SetMaxSpeed",             &ComponentPosition::SetMaxSpeed)

                .addFunction("IncrementPosition",     &ComponentPosition::IncrementMovement)
                .addFunction("IncrementMovement",     &ComponentPosition::IncrementMovement)
                .addFunction("IncrementAcceleration", &ComponentPosition::IncrementAcceleration)

                .addFunction("RoundPosition",         &ComponentPosition::RoundPosition)
                .addFunction("ChangeParent",          &ComponentPosition::ChangeParent)

                .addFunction("TranslateWorldToLocal", &ComponentPosition::TranslateWorldToLocal)
                .addFunction("TranslateLocalToWorld", &ComponentPosition::TranslateLocalToWorld)
            .endClass()

            .beginClass<ComponentCamera>("ComponentCamera")
                .addFunction("GetViewport",   &ComponentCamera::GetViewport)
            .endClass()

            .deriveClass<ParticleCreator, RenderableObject>("ParticleCreator")
                .addFunction("SetVelocity",             &ParticleCreator::SetVelocity)
                .addFunction("SetAcceleration",         &ParticleCreator::SetAcceleration)
                .addFunction("SetPosition",             &ParticleCreator::SetPosition)
                .addFunction("SetParticlesPerFrame",    &ParticleCreator::SetParticlesPerFrame)
                .addFunction("Start",                   &ParticleCreator::Start)
                .addFunction("SetColor",                &ParticleCreator::SetColor)
                .addFunction("SetScalingX",             &ParticleCreator::SetScalingX)
                .addFunction("SetScalingY",             &ParticleCreator::SetScalingY)
                .addFunction("SetVertexShaderCode",     &ParticleCreator::SetVertexShaderCode)
                .addFunction("SetFragmentShaderCode",   &ParticleCreator::SetFragmentShaderCode)
                .addFunction("SetShape",                &ParticleCreator::SetShape)
                .addFunction("SetEffect",               &ParticleCreator::SetEffect)
            .endClass()

            .deriveClass<RenderText, RenderableObject>("RenderText")
                .addFunction("ChangeText",      &RenderText::ChangeText)
                .addFunction("ChangePosition",  &RenderText::ChangePosition)

                .addFunction("SetColor",        &RenderText::SetColorI)
                .addFunction("SetX",            &RenderText::SetX)
                .addFunction("SetY",            &RenderText::SetY)
                .addFunction("SetW",            &RenderText::SetW)
                .addFunction("SetH",            &RenderText::SetH)

                .addFunction("GetX",            &RenderText::GetX)
                .addFunction("GetY",            &RenderText::GetY)
                .addFunction("GetW",            &RenderText::GetW)
                .addFunction("GetH",            &RenderText::GetH)
            .endClass()

            .deriveClass<RenderLine, RenderableObject>("RenderLine")
                .addFunction("ChangePosition",  &RenderLine::ChangePosition)

                .addFunction("SetColor",        &RenderLine::SetColorI)
                .addFunction("SetX1",           &RenderLine::SetX1)
                .addFunction("SetY1",           &RenderLine::SetY1)
                .addFunction("SetX2",           &RenderLine::SetX2)
                .addFunction("SetY2",           &RenderLine::SetY2)

                .addFunction("GetX1",           &RenderLine::GetX1)
                .addFunction("GetY1",           &RenderLine::GetY1)
                .addFunction("GetX2",           &RenderLine::GetX2)
                .addFunction("GetY2",           &RenderLine::GetY2)
            .endClass()

            .beginClass<LMap>("LMap")
                .addFunction("GetTileLayer",    &LMap::GetTileLayer)
                .addFunction("GetProperty",     &LMap::GetProperty)
                .addFunction("GetWidthTiles",        &LMap::GetWidthTiles)
                .addFunction("GetHeightTiles",       &LMap::GetHeightTiles)
                .addFunction("GetWidthPixels",        &LMap::GetWidthPixels)
                .addFunction("GetHeightPixels",       &LMap::GetHeightPixels)
            .endClass()

        .endNamespace()
    ;
}
