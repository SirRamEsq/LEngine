#include "LuaInterface.h"
#include "Exceptions.h"
#include "GameStates/GS_Script.h"
#include "Kernel.h"
#include "Resolution.h"
#include "StateManager.h"
#include "gui/imgui_LEngine.h"

#include <sstream>

void stackdump_g(lua_State *l, const std::string &logFile) {
  std::stringstream ss;
  ss << "\n";
  ss << "START STACK DUMP\n";
  int i;
  int top = lua_gettop(l);

  ss << "total in stack " << top << "\n";

  for (i = 1; i <= top; i++) { /* repeat for each level */
    int t = lua_type(l, i);
    ss << "---\n"; /* put a separator */
    switch (t) {
      case LUA_TSTRING: /* strings */
        ss << "string: " << lua_tostring(l, i) << "\n";
        break;
      case LUA_TBOOLEAN: /* booleans */
        ss << "boolean: " << (lua_toboolean(l, i) ? "true" : "false") << "\n";
        break;
      case LUA_TNUMBER: /* numbers */
        ss << "number: " << lua_tonumber(l, i) << "\n";
        break;
      default: /* other values */
        ss << "type is: " << lua_typename(l, t) << "\n";
        break;
    }
  }
  ss << "==========";
  LOG_DEBUG(ss.str());
}

std::unordered_map<std::string, luabridge::LuaRef> GetKeyValueMap(
    const luabridge::LuaRef &table) {
  using namespace luabridge;
  std::unordered_map<std::string, LuaRef> result;
  if (table.isNil()) {
    return result;
  }

  auto L = table.state();
  push(L, table);  // push table

  lua_pushnil(L);  // push nil, so lua_next removes it from stack and puts (k,
                   // v) on stack
  while (lua_next(L, -2) != 0) {  // -2, because we have table at -1
    if (lua_isstring(L, -2)) {    // only store stuff with string keys
      result.emplace(lua_tostring(L, -2), LuaRef::fromStack(L, -1));
    }
    lua_pop(L, 1);  // remove value, keep key for lua_next
  }

  lua_pop(L, 1);  // pop table
  return result;
}
int GetErrorInfo(lua_State *L) {
  lua_Debug d;
  lua_getstack(L, 1, &d);
  lua_getinfo(L, "Sln", &d);
  std::string err = lua_tostring(L, -1);
  lua_pop(L, 1);
  std::stringstream msg;
  msg << d.short_src << ":" << d.currentline;

  if (d.name != 0) {
    msg << "(" << d.namewhat << " " << d.name << ")";
  }
  msg << " " << err;

  std::stringstream ss;
  ss << "Lua Error: " << msg.str();
  LOG_ERROR(ss.str());
  lua_pushstring(L, msg.str().c_str());
  return 1;
}

using namespace luabridge;

// Base Entity needs to be inside Type_Dir
const std::string LuaInterface::BASE_SCRIPT_NAME = "__BaseEntity.lua";
const std::string LuaInterface::TYPE_DIR = "Common/";

const std::string LuaInterface::DEBUG_LOG = "LUA_INTERFACE";

// Debug table has all lua functionality
#ifdef DEBUG_MODE
const std::string LuaInterface::LUA_52_INTERFACE_ENV_TABLE =
    /*
    Could probably remove the dependency on LuaFileSystem entirely if I just
    pass
    the current directory from the engine
    */
    "lfs = require(\"lfs\")\n"
    "utilityPath		= lfs.currentdir() .. "
    "\"/Data/Resources/Scripts/Utility\" \n"
    "LEngineInitPath	= utilityPath .. \"/LEngineInit.lua\" \n"

    "_REQUIRE_FUNCTION = function (moduleName) \n"
    " local module = CPP.interface:ModuleLoad(moduleName) \n"
    " if module == nil then \n"
    "  local errorString = \"Couldn't load module named \" .. moduleName \n"
    "  CPP.interface:LogError(0, errorString) \n"
    "  return nil \n"
    " end \n"
    // wrap in a loader function, see
    // https://www.lua.org/manual/5.2/manual.html#pdf-require
    " loader = function(a1, a2) return module end \n"
    " return loader\n"
    "end \n"
    // remove default package loaders
    // package.loaders is lua 5.1
    // package.searchers is lua 5.2
    "package.loaders = {} \n"
    "package.loaders[1] = _REQUIRE_FUNCTION \n"
    "package.searchers = {} \n"
    "package.searchers[1] = _REQUIRE_FUNCTION \n"

    "newLoadFile = function(dir, env)\n"
    "if (env==nil) then return nil; end \n"
    "return loadfile(dir, \"bt\", env)\n"
    "end\n"
    "L_ENGINE_ENV = {}\n"
    "for k,v in pairs(_ENV)do\n"
    "L_ENGINE_ENV[k]=v\n"
    "end\n"
    "L_ENGINE_ENV._ENV = {}\n"
    "L_ENGINE_ENV.loadfile = newLoadFile\n"
    "L_ENGINE_ENV.utilityPath = utilityPath\n"
    "L_ENGINE_ENV.CPP = CPP\n"

    // run InitLEngine script using the restricted environment
    //"f1 = newLoadFile(LEngineInitPath, L_ENGINE_ENV) \n"
    "f1 = require(\"Utility/LEngineInit.lua\") \n"
    "NewLEngine = f1\n";
// Standard table has cherry picked functionality
#else
const std::string LuaInterface::LUA_52_INTERFACE_ENV_TABLE =
    // http://stackoverflow.com/questions/34388285/creating-a-secure-lua-sandbox

    // Wrapper around loadfile; Files can only load new files if they specify an
    // _ENV
    "lfs = require(\"lfs\")\n"
    "utilityPath		= lfs.currentdir() .. "
    "\"/Data/Resources/Scripts/Utility\" \n"
    "LEngineInitPath	= utilityPath .. \"/LEngineInit.lua\" \n"

    "_REQUIRE_FUNCTION = function (moduleName) \n"
    " local module = CPP.interface:ModuleLoad(moduleName) \n"
    " if module == nil then \n"
    "  local errorString = \"Couldn't load module named \" .. moduleName \n"
    "  CPP.interface:LogError(0, errorString) \n"
    "  return nil \n"
    " end \n"
    // wrap in a loader function, see
    // https://www.lua.org/manual/5.2/manual.html#pdf-require
    " loader = function(a1, a2) return module end \n"
    " return loader\n"
    "end \n"
    // remove default package loaders
    // package.loaders is lua 5.1
    // package.searchers is lua 5.2
    "package.loaders = {} \n"
    "package.loaders[1] = _REQUIRE_FUNCTION \n"
    "package.searchers = {} \n"
    "package.searchers[1] = _REQUIRE_FUNCTION \n"

    "newLoadFile = function(dir, env)\n"
    "if (env==nil) then return nil; end \n"
    "return loadfile(dir, \"bt\", env)\n"
    "end								"
    "								   \n"
    // Global table for this environment
    // Note, that in lua 5.2, there is nothing special about _G;
    //_ENV is the global table for a given function; _G is simply set at the
    // start of the entire lua state
    "L_ENGINE_ENV_G = {}\n"
    "L_ENGINE_ENV = {\n"
    // Global table between all scripts
    "_G = L_ENGINE_ENV_G,\n"
    // It is expected to have exposed CPP data (thus creating the CPP table)
    // before running this string
    "CPP = CPP,\n"
    "utilityPath= utilityPath,\n"
    // Use secure wrapper around loadfile
    "loadfile = newLoadFile, \n"
    "require = require,\n"
    // Built in Lua functions
    "ipairs = ipairs,\n"
    "next = next,\n"
    "pairs = pairs,	\n"
    "pcall = pcall,	\n"
    "tonumber = tonumber,\n"
    "tostring = tostring,\n"
    "type = type,\n"
    // Can set metatables, but not get
    "setmetatable = setmetatable,\n"
    "unpack = unpack,\n"

    "coroutine = {	\n"
    "create = coroutine.create, resume = coroutine.resume,\n"
    "running = coroutine.running, status = coroutine.status,\n"
    "wrap = coroutine.wrap\n"
    "},\n"

    "string = {	\n"
    "byte = string.byte, char = string.char, find = string.find,\n"
    "format = string.format, gmatch = string.gmatch,"
    "gsub = string.gsub,	\n"
    "len = string.len, lower = string.lower, match = string.match,	\n"
    "rep = string.rep, reverse = string.reverse, sub = string.sub,	\n"
    "upper = string.upper\n"
    "},\n"

    "table = {\n"
    "insert = table.insert, maxn = table.maxn, remove = table.remove,	\n"
    "sort = table.sort\n"
    "},\n"

    "math = {\n"
    "abs = math.abs, acos = math.acos, asin = math.asin,		\n"
    "atan = math.atan, atan2 = math.atan2, ceil = math.ceil, cos = \n"
    "math.cos,	\n"
    "cosh = math.cosh, deg = math.deg, exp = math.exp, floor = \n"
    "math.floor,	\n"
    "fmod = math.fmod, frexp = math.frexp, huge = math.huge,		\n"
    "ldexp = math.ldexp, log = math.log, log10 = math.log10, max = \n"
    "math.max,\n"
    "min = math.min, modf = math.modf, pi = math.pi, pow = math.pow,\n"
    "rad = math.rad, randomseed = math.randomseed, random = math.random, sin = "
    "\n"
    "math.sin, sinh = math.sinh,	\n"
    "sqrt = math.sqrt, tan = math.tan, tanh = math.tanh\n"
    "},\n"

    "os = { date = os.date, clock = os.clock, difftime = os.difftime, time = \n"
    "os.time },	\n"

    "select=select, \n"
    "}\n"
    "f1 = require(\"Utility/LEngineInit.lua\") \n"
    "NewLEngine = f1\n";

#endif

LuaInterface::LuaInterface(GameState *state) : parentState(state) {
  lState = luaL_newstate();
  errorCallbackFunction = NULL;

  if (lState == NULL) {
    std::stringstream ss;
    ss << "Lua state couldn't be created";
    LOG_FATAL(ss.str());
    return;
  }

  luaL_openlibs(lState);
  // Create 'CPP' table referenced by LUA_52_INTERFACE_ENV_TABLE
  ExposeCPP();

  LuaRef cppTable = getGlobal(lState, "CPP");
  if (cppTable.isNil()) {
    LOG_FATAL("CPP is nil");
  }
  if (!cppTable.isTable()) {
    LOG_FATAL("CPP is not table");
  }

  cppTable["interface"] = this;

  LuaRef interfaceInstance = cppTable["interface"];
  if (interfaceInstance.isNil()) {
    LOG_FATAL("interface instance is nil");
  }

  auto error = luaL_dostring(lState, LUA_52_INTERFACE_ENV_TABLE.c_str());
  if (error != 0) {
    std::stringstream ss;
    ss << "Lua String could not be run \n"
       << LUA_52_INTERFACE_ENV_TABLE << "\n"
       << " | Error code is: " << error << " "
       << "   ...Error Message is " << lua_tostring(lState, -1);
    LOG_FATAL(ss.str());
    lua_pop(lState, -1);  // pop error message
    // GetErrorInfo(lState);
    return;
  }

  LuaRef engineFunction = getGlobal(lState, "NewLEngine");
  if (engineFunction.isNil()) {
    LOG_FATAL("NewLEngine is nil");
  }
  if (!engineFunction.isFunction()) {
    LOG_FATAL("NewLEngine is not function");
  }

  // GetBaseEntity Creator
  std::string scriptName = TYPE_DIR + BASE_SCRIPT_NAME;
  baseScript = K_ScriptMan.GetItem(scriptName);
  if (baseScript == NULL) {
    K_ScriptMan.LoadItem(scriptName, scriptName);
    baseScript = K_ScriptMan.GetItem(scriptName);
    if (baseScript == NULL) {
      LOG_ERROR("Cannot load base entity lua script");
      LOG_ERROR(scriptName);
    }
  }
  if (baseScript != NULL) {
    try {
      int functionReference = RunScriptLoadFromChunk(baseScript);

      // saves function for later use
      baseLuaClass = functionReference;
    } catch (LEngineException e) {
      LOG_FATAL(e.what());
    }
  }
}

LuaInterface::~LuaInterface() {
  // close state
  lua_close(lState);
}

LuaRef LuaInterface::ModuleLoad(const std::string &moduleName) {
  LuaRef nil(lState);

  // step 1, has the packgae already been loaded?
  auto i = mLoadedModules.find(moduleName);
  if (i != mLoadedModules.end()) {
    auto module = i->second;
    // push reference and return
    lua_rawgeti(lState, LUA_REGISTRYINDEX, module);  // stack size + 1
    LuaRef moduleRef = LuaRef::fromStack(lState, -1);
    lua_pop(lState, 1);  // stack size + 0
    return moduleRef;
  }

  // step 2, can we find a RSC_Script matching the moduleName?
  auto script = K_ScriptMan.GetLoadItem(moduleName, moduleName);
  if (script == NULL) {
    return nil;
  }

  // step 3; can the script be run?
  int error =
      luaL_loadbufferx(lState, script->script.c_str(), script->script.length(),
                       script->scriptName.c_str(), "bt");
  // stack size + 1
  if (error != 0) {
    std::stringstream ss;
    ss << "Script [" << script->scriptName
       << "] could not be loaded from require\n"
       << "   ...Error code " << error << "\n"
       << "   ...Error Message is " << lua_tostring(lState, -1);
    LOG_ERROR(ss.str());
    lua_pop(lState, 1);  // stack size + 0
    return nil;
  }

  // push environment onto stack
  lua_getglobal(lState, "L_ENGINE_ENV");  // stack size + 2
  // pop environment and assign to upvalue#1 (the func environment)
  lua_setupvalue(lState, -2, 1);  // stack size + 1

  // Pcall will run the module and push the return value to the top
  if (int error = lua_pcall(lState, 0, 1, 0) != 0) {  // stack size + 1
    std::stringstream ss;
    ss << "Script [" << script->scriptName
       << "] could not be run from require\n"
       << "   ...Error code " << error << "\n"
       << "   ...Error Message is " << lua_tostring(lState, -1);
    LOG_ERROR(ss.str());
    lua_pop(lState, 1);  // stack size + 0
    return nil;
  }

  // Gets reference to the top of the stack and pops
  int module = luaL_ref(lState, LUA_REGISTRYINDEX);  // stack size + 0
  // store reference for later
  mLoadedModules[moduleName] = module;

  // push reference and return
  lua_rawgeti(lState, LUA_REGISTRYINDEX, module);  // stack size + 1
  LuaRef moduleRef = LuaRef::fromStack(lState, -1);
  lua_pop(lState, 1);  // stack size + 0

  return moduleRef;
}

int LuaInterface::RunScriptLoadFromChunk(const RSC_Script *script) {
  //////////////
  // Load Chunk//
  //////////////
  int error =
      luaL_loadbufferx(lState, script->script.c_str(), script->script.length(),
                       script->scriptName.c_str(), "bt");
  if (error != 0) {
    std::stringstream errorMsg;
    errorMsg << "Script [" << script->scriptName
             << "] could not be loaded from C++\n"
             << "   ...Error code " << error << "\n"
             << "   ...Error Message is " << lua_tostring(lState, -1);
    // completely clear the stack before return
    lua_settop(lState, 0);
    throw LEngineException(errorMsg.str());
  }

  // push environment onto stack
  lua_getglobal(lState, "L_ENGINE_ENV");
  // pop environment and assign to upvalue#1 (the func environment)
  lua_setupvalue(lState, -2, 1);

  if (int error = lua_pcall(lState, 0, 1, 0) != 0) {
    // Calling pcall pops the function and its args [#args is arg 1] from the
    // stack
    // The result [# of returns is arg 2] is pushed on the stack
    // should be a function that can create
    // a table with "Update", "Init", etc... methods
    std::stringstream errorMsg;
    errorMsg << "Script [" << script->scriptName
             << "] could not be run from C++\n"
             << "   ...Error code " << error << "\n"
             << "   ...Error Message is " << lua_tostring(lState, -1);
    // completely clear the stack before return
    lua_settop(lState, 0);
    throw LEngineException(errorMsg.str());
  }

  // Get top of stack (returned value from pcall)
  int stackTop = lua_gettop(lState);
  if (lua_isfunction(lState, stackTop) == false) {
    std::stringstream errorMsg;
    errorMsg << "Returned value is not function in script ["
             << script->scriptName << "] \n"
             << " ...Type is: ["
             << lua_typename(lState, lua_type(lState, stackTop)) << "]";
    // completely clear the stack before return
    lua_settop(lState, 0);
    throw LEngineException(errorMsg.str());
  }

  // pops function off of stack
  return luaL_ref(lState, LUA_REGISTRYINDEX);
}

int LuaInterface::LookupFunction(const RSC_Script *script) {
  int functionReference = 0;
  auto classDefinition = mFunctionLookup.find(script);
  if (classDefinition != mFunctionLookup.end()) {
    functionReference = classDefinition->second;
  }

  else {
    // Load chunk if it hasn't been loaded before
    try {
      functionReference = RunScriptLoadFromChunk(script);
    } catch (LEngineException e) {
      LOG_FATAL(e.what());
      throw e;
    }

    // saves function for later use
    mFunctionLookup[script] = functionReference;
  }
  return functionReference;
}

///////////
// General//
///////////

// Clears stack
bool LuaInterface::RunScript(EID id, std::vector<const RSC_Script *> scripts,
                             MAP_DEPTH depth, EID parent,
                             const std::string &name, const TiledObject *obj,
                             LuaRef *initTable) {
  if (scripts.empty()) {
    std::stringstream ss;
    ss << "Couldn't run script for entity with EID " << id
       << " as No scripts were passed to be run";
    LOG_ERROR(ss.str());
    throw LEngineException(ss.str());
    return false;
  }
  // check if entity has script component
  ComponentScript *scriptComponent = parentState->comScriptMan.GetComponent(id);
  if (scriptComponent == NULL) {
    std::stringstream ss;
    ss << "Couldn't run script for entity with EID " << id
       << " as this entity does not have a script component";
    LOG_ERROR(ss.str());
    throw LEngineException(ss.str());
  }

  // Base Script will be the first one run
  if (baseScript != NULL) {
    scripts.insert(scripts.begin(), baseScript);
  }

  // Data structure containing Luafunction ID values
  std::unordered_map<const RSC_Script *, int> scriptFunctions;
  // this datastructure defines the order in which scripts are run
  std::vector<const RSC_Script *> scriptOrder;

  // Use Rbegin and REnd so that type functions are called in the order
  // that they are declared in
  // last one declared is at the bottom of the stack
  // First one declared is at the top
  for (auto i = scripts.rbegin(); i != scripts.rend(); i++) {
    auto script = *i;
    if (script != NULL) {
      auto scriptFunction = LookupFunction(script);
      scriptFunctions[script] = (scriptFunction);
      // push front
      scriptOrder.insert(scriptOrder.begin(), script);
      if (scriptFunction != -1) {
        // push type function along with base class argument and call function
        lua_rawgeti(lState, LUA_REGISTRYINDEX, scriptFunction);
        LOG_TRACE("Pushed Type " + script->scriptName);
      }
    }
  }

  // Push nil as the first table argument to the base script
  lua_pushnil(lState);

  std::stringstream fullyQualifiedScriptName;
  // Call previously acquired script function ID values
  for (auto i = scriptOrder.begin(); i != scriptOrder.end(); i++) {
    auto scriptFunction = scriptFunctions[*i];
    auto script = *i;
    auto scriptName = script->scriptName;
    fullyQualifiedScriptName << scriptName;
    if (!IteratorIsLast(i, scriptOrder)) {
      fullyQualifiedScriptName << " : ";
    }
    if (scriptFunction != -1) {
      // Call function with 1 arg and place returned table at the top of the
      // stack
      if (int error = lua_pcall(lState, 1, 1, 0) != 0) {
        std::stringstream ss;
        ss << "type did not return a callable function\n"
           << "   ...Error code " << error << "\n";
        ss << "Error String is '" << lua_tostring(lState, -1) << "'\n";
        ss << "ScriptName is: " << scriptName;
        // completely clear the stack before return
        lua_settop(lState, 0);
        LOG_ERROR(ss.str());

        throw LEngineException(ss.str());
        return false;
      }
    }
  }

  // the top of the stack should now contain the final table value for this
  // script
  int stackTop = lua_gettop(lState);
  if (!lua_istable(lState, stackTop)) {
    // Returned value isn't table; cannot be used
    std::stringstream errorMsg;
    errorMsg << "Returned value from lua function is not a table in script ["
             << fullyQualifiedScriptName.str() << "] with EID [" << id << "] \n"
             << " ...Type is: ["
             << lua_typename(lState, lua_type(lState, stackTop)) << "]";
    LOG_ERROR(errorMsg.str());
    // completely clear the stack before return
    lua_settop(lState, 0);
    return false;
  }

  // Set 'name+eid" to the returned table containing the 'Update', 'Init',
  // etc... functions
  // store temporarily in returnedTableName  (will later set to nil)
  std::stringstream returnedTableName;
  returnedTableName << "_LE_TABLE_" << name << id;
  lua_setglobal(lState, returnedTableName.str().c_str());  // assign and pop
                                                           // value

  ////////////////////////////////////////////////////////////
  // Assign Instance of LEngine to script for starting values//
  ////////////////////////////////////////////////////////////
  try {
    // Create new Instance
    LuaRef engineRef = getGlobal(lState, "NewLEngine");
    LuaRef engineTableRef = engineRef();

    engineTableRef["Initialize"](id, name, fullyQualifiedScriptName.str(),
                                 depth, parent, Kernel::IsInDebugMode());

    // Assign Instance to generated table
    LuaRef returnedTable = getGlobal(lState, returnedTableName.str().c_str());
    returnedTable["LEngineData"] = engineTableRef;
  } catch (std::exception e) {
    LOG_ERROR("Couldn't Run LEngineInit; ");
    LOG_ERROR(e.what());
  }

  //////////////////////////////////////////////////
  // Assign values from optional args to the script//
  //////////////////////////////////////////////////

  if (obj != NULL) {
    LuaRef returnedTable = getGlobal(lState, returnedTableName.str().c_str());
    LuaRef lengineData = returnedTable["LEngineData"];
    LuaRef initTable = lengineData["InitializationTable"];
    for (auto i = obj->properties.bools.begin();
         i != obj->properties.bools.end(); i++) {
      initTable[i->first] = i->second;
    }
    for (auto i = obj->properties.ints.begin(); i != obj->properties.ints.end();
         i++) {
      initTable[i->first] = i->second;
    }
    for (auto i = obj->properties.floats.begin();
         i != obj->properties.floats.end(); i++) {
      initTable[i->first] = i->second;
    }
    for (auto i = obj->properties.strings.begin();
         i != obj->properties.strings.end(); i++) {
      initTable[i->first] = i->second;
    }
  }

  if (initTable != NULL) {
    if (initTable->isNil() == false) {
      LuaRef returnedTable = getGlobal(lState, returnedTableName.str().c_str());
      LuaRef lengineData = returnedTable["LEngineData"];
      lengineData["InitializationTable"] = (*initTable);
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////
  // Create new script component and assign reference to the new table to the
  // component//
  //////////////////////////////////////////////////////////////////////////////////////
  LuaRef returnedTable = getGlobal(lState, returnedTableName.str().c_str());

  // Initialize Component which has already been created
  scriptComponent->SetScriptPointerOnce(returnedTable, &scripts);
  scriptComponent->scriptName = fullyQualifiedScriptName.str();
  scriptComponent->RunFunction("Initialize");

  // set returnedTableName to nil
  lua_pushnil(lState);
  lua_setglobal(lState, returnedTableName.str().c_str());

  return true;
}

const RSC_Sprite *LuaInterface::LoadSprite(const std::string &sprPath) {
  const RSC_Sprite *sprite = K_SpriteMan.GetItem(sprPath);
  if (sprite == NULL) {
    if (K_SpriteMan.LoadItem(sprPath, sprPath) == false) {
      LOG_ERROR("LuaInterface::LoadSprite; Couldn't Load Sprite Named: " +
                sprPath);
      return NULL;
    }
    sprite = K_SpriteMan.GetItem(sprPath);
  }
  return sprite;
}

void LuaInterface::ListenForInput(EID id, const std::string &inputName) {
  ComponentInput *comInput = (parentState->comInputMan.GetComponent(id));
  if (comInput == NULL) {
    parentState->comInputMan.AddComponent(id);
    comInput = (parentState->comInputMan.GetComponent(id));
    if (comInput == NULL) {
      std::stringstream ss;
      ss << "Couldn't listen for input for script with eid " << id
         << " no input component";
      LOG_ERROR(ss.str());
      return;
    }
  }
  comInput->ListenForInput(inputName);
}

std::string LuaInterface::GenerateLogEntry(EID id, const std::string &error) {
  lua_Debug ar;
  auto L = GetState();
  lua_getstack(L, 1, &ar);
  lua_getinfo(L, "nSl", &ar);
  int line = ar.currentline;

  ComponentScript *component = parentState->comScriptMan.GetComponent(id);
  std::stringstream ss;

  std::string name = "???";
  if (component != NULL) {
    name = component->scriptName;
  }
  ss << "[LUA"
     << " | " << name << " | EID: " << id << " | Line: " << line << "]"
     << std::endl
     << "        " << error;

  if (errorCallbackFunction != NULL) {
    errorCallbackFunction(id, error);
  }

  return ss.str();
}

void LuaInterface::LogFatal(EID id, const std::string &message) {
  LOG_FATAL(GenerateLogEntry(id, message));
}
void LuaInterface::LogError(EID id, const std::string &message) {
  LOG_ERROR(GenerateLogEntry(id, message));
}
void LuaInterface::LogWarn(EID id, const std::string &message) {
  LOG_WARN(GenerateLogEntry(id, message));
}
void LuaInterface::LogInfo(EID id, const std::string &message) {
  LOG_INFO(GenerateLogEntry(id, message));
}
void LuaInterface::LogDebug(EID id, const std::string &message) {
  LOG_DEBUG(GenerateLogEntry(id, message));
}
void LuaInterface::LogTrace(EID id, const std::string &message) {
  LOG_TRACE(GenerateLogEntry(id, message));
}

void LuaInterface::PlaySound(const std::string &sndName, int volume) {
  SoundEvent snd(sndName, volume);
  if (snd.sound != NULL) {
    K_AudioSub.PushSoundEvent(snd);
  }
}

void LuaInterface::PlayMusic(const std::string &musName, int volume,
                             int loops) {
  auto music = K_MusicMan.GetLoadItem(musName, musName);
  if (music == NULL) {
    return;
  }
  K_AudioSub.PlayMusic(music, volume, loops);
}

//////////////
// Components//
//////////////
ComponentPosition *LuaInterface::GetPositionComponent(const EID &id) {
  if (parentState->comPosMan.HasComponent(id) == false) {
    parentState->comPosMan.AddComponent(id);
  }
  return parentState->comPosMan.GetComponent(id);
}
ComponentSprite *LuaInterface::GetSpriteComponent(const EID &id) {
  if (parentState->comSpriteMan.HasComponent(id) == false) {
    parentState->comSpriteMan.AddComponent(id);
  }
  return parentState->comSpriteMan.GetComponent(id);
}
ComponentCollision *LuaInterface::GetCollisionComponent(const EID &id) {
  if (parentState->comCollisionMan.HasComponent(id) == false) {
    parentState->comCollisionMan.AddComponent(id);
  }
  return parentState->comCollisionMan.GetComponent(id);
}
ComponentParticle *LuaInterface::GetParticleComponent(const EID &id) {
  if (parentState->comParticleMan.HasComponent(id) == false) {
    parentState->comParticleMan.AddComponent(id);
  }
  return parentState->comParticleMan.GetComponent(id);
}
ComponentCamera *LuaInterface::GetCameraComponent(const EID &id) {
  if (parentState->comCameraMan.HasComponent(id) == false) {
    parentState->comCameraMan.AddComponent(id);
  }
  return parentState->comCameraMan.GetComponent(id);
}
ComponentLight *LuaInterface::GetLightComponent(const EID &id) {
  if (parentState->comLightMan.HasComponent(id) == false) {
    parentState->comLightMan.AddComponent(id);
  }
  return parentState->comLightMan.GetComponent(id);
}

bool LuaInterface::HasPositionComponent(const EID &id) {
  return parentState->comPosMan.HasComponent(id);
}
bool LuaInterface::HasSpriteComponent(const EID &id) {
  return parentState->comSpriteMan.HasComponent(id);
}
bool LuaInterface::HasCollisionComponent(const EID &id) {
  return parentState->comCollisionMan.HasComponent(id);
}
bool LuaInterface::HasParticleComponent(const EID &id) {
  return parentState->comParticleMan.HasComponent(id);
}
bool LuaInterface::HasCameraComponent(const EID &id) {
  return parentState->comCameraMan.HasComponent(id);
}
bool LuaInterface::HasLightComponent(const EID &id) {
  return parentState->comLightMan.HasComponent(id);
}
////////////
// Entities//
////////////
const std::vector<EID> *LuaInterface::EntityGetFromName(
    const std::string &name) {
  auto eids = parentState->GetEIDFromName(name);
  return eids;
}

luabridge::LuaRef LuaInterface::EntityGetInterface(const EID &id) {
  return (parentState->comScriptMan.GetComponent(id))->GetEntityInterface();
}

Coord2df LuaInterface::EntityGetPositionWorld(EID entity) {
  ComponentPosition *pos = (parentState->comPosMan.GetComponent(entity));
  if (pos == NULL) {
    std::stringstream ss;
    ss << "EntityGetPositionWorld was passed entity id " << entity
       << " Which does not exist";
    LOG_ERROR(ss.str());
    return Coord2df(0, 0);
  }
  return pos->GetPositionWorld();
}

Coord2df LuaInterface::EntityGetMovement(EID entity) {
  return (parentState->comPosMan.GetComponent(entity))->GetMovement();
}

EID LuaInterface::EntityNew(std::string name, int x, int y, MAP_DEPTH depth,
                            EID parent, luabridge::LuaRef scripts,
                            luabridge::LuaRef propertyTable) {
  std::vector<std::string> scriptNames;

  // if passed type is a simple string
  if (scripts.type() == LUA_TSTRING) {
    auto type = scripts.cast<std::string>();
    scriptNames.push_back(type);
  }

  // if passed type is a table
  else if (scripts.type() == LUA_TTABLE) {
    auto kvPairs = GetKeyValueMap(scripts);
    for (auto i = kvPairs.begin(); i != kvPairs.end(); i++) {
      auto tempRef = i->second;
      if (tempRef.type() == LUA_TSTRING) {
        auto type = tempRef.cast<std::string>();
        scriptNames.push_back(type);
      } else {
        LOG_WARN(
            "LuaRef 'scripts' was passed a table containing a value that is "
            "not "
            "a string");
      }
    }
  }

  else {
    LOG_WARN("LuaRef 'scripts' was passed a type not supported");
  }

  auto packet = std::make_unique<EntityCreationPacket>(
      scriptNames, Coord2df(x, y), depth, parent, name, propertyTable);

  return parentState->CreateLuaEntity(std::move(packet));
}

void LuaInterface::EntityDelete(EID entity) {
  parentState->entityMan.DeleteEntity(entity);
}

/////////////
// Rendering//
/////////////
RenderLine *LuaInterface::RenderObjectLine(EID selfID, int x, int y, int xx,
                                           int yy) {
  ComponentScript *script = (parentState->comScriptMan.GetComponent(selfID));
  return script->RenderObjectLine(x, y, xx, yy);
}
void LuaInterface::RenderObjectDelete(EID selfID, RenderableObject *obj) {
  ComponentScript *script = (parentState->comScriptMan.GetComponent(selfID));
  script->RenderObjectDelete(obj);
}

//////////
// Events//
//////////
void LuaInterface::EventLuaObserveEntity(EID listenerID, EID senderID) {
  mEntitiesToObserve[listenerID].insert(senderID);
}

void LuaInterface::ProcessObservers() {
  for (auto i = mEntitiesToObserve.begin(); i != mEntitiesToObserve.end();
       i++) {
    for (auto ii = i->second.begin(); ii != i->second.end(); ii++) {
      auto senderID = *ii;
      auto listenerID = i->first;

      // Get the script that the listener wants to hear
      ComponentScript *senderScript =
          (parentState->comScriptMan.GetComponent(senderID));
      ComponentScript *listenerScript =
          (parentState->comScriptMan.GetComponent(listenerID));

      if (senderScript == NULL) {
        std::stringstream ss;
        ss << "Error: In function EventLuaObserveEntity; Cannot find entity "
           << "with id: " << (senderID);
        LOG_ERROR(ss.str())
        continue;
      }
      if (listenerScript == NULL) {
        std::stringstream ss;
        ss << "Error: In function EventLuaObserveEntity; Cannot find entity "
           << "with id: " << (listenerID);
        LOG_ERROR(ss.str())
        continue;
      }

      // Add the listener to the sender's list of observers
      senderScript->EventLuaAddObserver(listenerScript);
    }
  }
  mEntitiesToObserve.clear();
}

void LuaInterface::Update() { ProcessObservers(); }

void LuaInterface::EventLuaBroadcastEvent(EID senderID,
                                          const std::string &event) {
  ComponentScript *script = (parentState->comScriptMan.GetComponent(senderID));

  script->EventLuaBroadcastEvent(event);
}

void LuaInterface::EventLuaSendToObservers(EID senderID,
                                           const std::string &event) {
  ComponentScript *script = (parentState->comScriptMan.GetComponent(senderID));

  script->EventLuaSendToObservers(event);
}

void LuaInterface::EventLuaSendEvent(EID senderID, EID recieverID,
                                     const std::string &event) {
  Event e(senderID, recieverID, Event::MSG::LUA_EVENT, event);

  ComponentScript *script = parentState->comScriptMan.GetComponent(recieverID);
  if (script == NULL) {
    return;
  }
  script->HandleEvent(&e);
}

///////////
// Handles//
///////////
RSC_Map *LuaInterface::GetMap() { return parentState->GetCurrentMap(); }

/////////
// Other//
/////////
void LuaInterface::SetParent(EID child, EID parent) {
  parentState->entityMan.SetParent(child, parent);
}

GS_Script *LuaInterface::PushState(const std::string &scriptPath) {
  if (parentState->IsLuaState() == true) {
    return ((GS_Script *)(parentState))->PushState(scriptPath);
  }
  return NULL;
}

void LuaInterface::PopState() { Kernel::stateMan.PopState(); }

void LuaInterface::SwapState(const std::string &scriptPath) {
  const RSC_Script *script = K_ScriptMan.GetLoadItem(scriptPath, scriptPath);
  Kernel::stateMan.SwapState(std::make_shared<GS_Script>(&Kernel::stateMan),
                             script);
}

bool LuaInterface::LoadMap(const std::string &mapPath,
                           unsigned int entranceID) {
  const RSC_Map *m = K_MapMan.GetLoadItem(mapPath, mapPath);
  if (m == NULL) {
    std::stringstream ss;
    ss << "Map '" << mapPath << "' not found";
    LOG_ERROR(ss.str());
    return false;
  }
  parentState->SetMapNextFrame(m, entranceID);
  return true;
}

void LuaInterface::RemapInputToNextKeyPress(const std::string &key) {
  Kernel::stateMan.inputManager->RemapKey(key);
}

Coord2df LuaInterface::GetMousePosition() {
  return Kernel::inputManager.GetMousePosition();
}

float LuaInterface::GetMouseWheel() {
  return Kernel::inputManager.GetMouseWheel();
}

bool LuaInterface::GetMouseButtonLeft() {
  return Kernel::inputManager.GetMouseButtonLeft();
}

bool LuaInterface::GetMouseButtonRight() {
  return Kernel::inputManager.GetMouseButtonRight();
}

bool LuaInterface::GetMouseButtonMiddle() {
  return Kernel::inputManager.GetMouseButtonMiddle();
}

Coord2df LuaInterface::GetResolution() { return Resolution::GetResolution(); }

Coord2df LuaInterface::GetVirtualResolution() {
  return Resolution::GetVirtualResolution();
}

void LuaInterface::SimulateKeyPress(const std::string &keyName) {
  Kernel::inputManager.SimulateKeyPress(keyName);
}

void LuaInterface::SimulateKeyRelease(const std::string &keyName) {
  Kernel::inputManager.SimulateKeyRelease(keyName);
}

bool LuaInterface::RecordKeysBegin() { return false; }

bool LuaInterface::RecordKeysEnd() { return false; }

GS_Script *LuaInterface::GetCurrentGameState() {
  if (parentState->IsLuaState() == true) {
    return (GS_Script *)parentState;
  }
  return NULL;
}

void LuaInterface::DeleteLayer(TiledLayerGeneric *layer) {
  parentState->DeleteMapLayer(layer);
}

void LuaInterface::SetAmbientLight(float r, float g, float b) {
  Vec3 color(r, g, b);
  parentState->comLightMan.SetAmbientLight(color);
}

LB_VEC_WRAPPER<TiledLayerGeneric *> LuaInterface::GetLayersWithProperty(
    const std::string &name, luabridge::LuaRef value) {
  auto m = GetMap();
  auto type = value.type();
  if (m != NULL) {
    if (type == LUA_TBOOLEAN) {
      bool boolValue = (value.cast<bool>() ? "true" : "false");
      auto layers = m->GetLayersWithProperty(name, boolValue);
      return LB_VEC_WRAPPER<TiledLayerGeneric *>(&layers);
    } else if (type == LUA_TNUMBER) {
      // case LUA_TNUMBER:
      // auto boolValue = (value.cast<bool>() ? "true" : "false");
      // return LB_VEC_WRAPPER<TiledLayerGeneric *>(
      // m->GetLayersWithProperty(name, boolValue));
      // break;
    } else if (type == LUA_TSTRING) {
      auto stringValue = value.cast<std::string>();
      // return m->GetLayersWithProperty(name, stringValue);
    }
  }

  // return std::vector<TiledLayerGeneric *>();
  return LB_VEC_WRAPPER<TiledLayerGeneric *>();
}

void LuaInterface::ExposeCPP() {
  typedef std::vector<EID> VectorEID;
  typedef std::vector<EID> VectorString;
  typedef LB_VEC_WRAPPER<TiledLayerGeneric *> VectorLayer;
  /*
   * if a const pointer is passed to lua
   * it WILL NOT CONTAIN non-const methods
   * it will be as if they aren't even declared
   */
  getGlobalNamespace(lState)                     // global namespace to lua
      .beginNamespace("CPP")                     //'CPP' table
      .beginClass<LuaInterface>("LuaInterface")  // define class object
      .addFunction("RenderObjectDelete", &LuaInterface::RenderObjectDelete)
      .addFunction("RenderObjectLine", &LuaInterface::RenderObjectLine)

      .addFunction("ListenForInput", &LuaInterface::ListenForInput)
      .addFunction("PlaySound", &LuaInterface::PlaySound)

      .addFunction("LoadSprite", &LuaInterface::LoadSprite)

      .addFunction("LogFatal", &LuaInterface::LogFatal)
      .addFunction("LogError", &LuaInterface::LogError)
      .addFunction("LogWarn", &LuaInterface::LogWarn)
      .addFunction("LogInfo", &LuaInterface::LogInfo)
      .addFunction("LogDebug", &LuaInterface::LogDebug)
      .addFunction("LogTrace", &LuaInterface::LogTrace)

      .addFunction("GetSpriteComponent", &LuaInterface::GetSpriteComponent)
      .addFunction("GetCollisionComponent",
                   &LuaInterface::GetCollisionComponent)
      .addFunction("GetPositionComponent", &LuaInterface::GetPositionComponent)
      .addFunction("GetParticleComponent", &LuaInterface::GetParticleComponent)
      .addFunction("GetCameraComponent", &LuaInterface::GetCameraComponent)
      .addFunction("GetLightComponent", &LuaInterface::GetLightComponent)

      .addFunction("HasSpriteComponent", &LuaInterface::HasSpriteComponent)
      .addFunction("HasCollisionComponent",
                   &LuaInterface::HasCollisionComponent)
      .addFunction("HasPositionComponent", &LuaInterface::HasPositionComponent)
      .addFunction("HasParticleComponent", &LuaInterface::HasParticleComponent)
      .addFunction("HasCameraComponent", &LuaInterface::HasCameraComponent)
      .addFunction("HasLightComponent", &LuaInterface::HasLightComponent)

      .addFunction("EntityNew", &LuaInterface::EntityNew)
      .addFunction("EntityGetInterface", &LuaInterface::EntityGetInterface)
      .addFunction("EntityDelete", &LuaInterface::EntityDelete)
      .addFunction("EntityGetFromName", &LuaInterface::EntityGetFromName)
      .addFunction("EntityGetPositionWorld",
                   &LuaInterface::EntityGetPositionWorld)
      .addFunction("EntityGetMovement", &LuaInterface::EntityGetMovement)

      .addFunction("EventLuaObserveEntity",
                   &LuaInterface::EventLuaObserveEntity)
      .addFunction("EventLuaSendEvent", &LuaInterface::EventLuaSendEvent)
      .addFunction("EventLuaBroadcastEvent",
                   &LuaInterface::EventLuaBroadcastEvent)
      .addFunction("EventLuaSendToObservers",
                   &LuaInterface::EventLuaSendToObservers)

      .addFunction("GetMap", &LuaInterface::GetMap)
      .addFunction("SetParent", &LuaInterface::SetParent)

      .addFunction("GetMousePosition", &LuaInterface::GetMousePosition)
      .addFunction("GetMouseWheel", &LuaInterface::GetMouseWheel)
      .addFunction("GetMouseButtonLeft", &LuaInterface::GetMouseButtonLeft)
      .addFunction("GetMouseButtonRight", &LuaInterface::GetMouseButtonRight)
      .addFunction("GetMouseButtonMiddle", &LuaInterface::GetMouseButtonMiddle)

      .addFunction("GetResolution", &LuaInterface::GetResolution)
      .addFunction("GetVirtualResolution", &LuaInterface::GetVirtualResolution)

      .addFunction("PushState", &LuaInterface::PushState)
      .addFunction("PopState", &LuaInterface::PopState)
      .addFunction("SwapState", &LuaInterface::SwapState)
      .addFunction("LoadMap", &LuaInterface::LoadMap)
      .addFunction("RemapInputToNextKeyPress",
                   &LuaInterface::RemapInputToNextKeyPress)
      .addFunction("SimulateKeyPress", &LuaInterface::SimulateKeyPress)
      .addFunction("SimulateKeyRelease", &LuaInterface::SimulateKeyRelease)
      .addFunction("SetAmbientLight", &LuaInterface::SetAmbientLight)
      .addFunction("RecordKeysBegin", &LuaInterface::RecordKeysBegin)
      .addFunction("RecordKeysEnd", &LuaInterface::RecordKeysEnd)

      .addFunction("ModuleLoad", &LuaInterface::ModuleLoad)

      //.addFunction("DeleteLayer", &LuaInterface::DeleteLayer)

      .addFunction("GetCurrentGameState", &LuaInterface::GetCurrentGameState)
      .addFunction("GetLayersWithProperty",
                   &LuaInterface::GetLayersWithProperty)
      .endClass()

      .beginClass<TiledLayerGeneric>("TiledLayerGeneric")
      .addFunction("SetAlpha", &TiledLayerGeneric::SetAlpha)
      .addFunction("GetAlpha", &TiledLayerGeneric::GetAlpha)
      .addFunction("GetFlags", &TiledLayerGeneric::GetFlags)
      .endClass()

      .deriveClass<TiledTileLayer, TiledLayerGeneric>("TiledTileLayer")
      .addFunction("GetTileProperty", &TiledTileLayer::GetTileProperty)
      .addFunction("UsesHMaps", &TiledTileLayer::UsesHMaps)
      .addFunction("GetTile", &TiledTileLayer::GetTile)
      .addFunction("SetTile", &TiledTileLayer::SetTile)
      .addFunction("HasTile", &TiledTileLayer::HasTile)
      .addFunction("UpdateRenderArea", &TiledTileLayer::UpdateRenderArea)
      .endClass()

      .beginClass<RSC_Sprite>("RSC_Sprite")
      .addFunction("GetOrigin", &RSC_Sprite::GetOrigin)
      .addFunction("SetOrigin", &RSC_Sprite::SetOrigin)
      .addFunction("SetColorKey", &RSC_Sprite::SetColorKey)
      .addFunction("GetName", &RSC_Sprite::GetName)
      .addFunction("Width", &RSC_Sprite::GetWidth)
      .addFunction("Height", &RSC_Sprite::GetHeight)
      .endClass()

      .beginClass<TColPacket>("TColPacket")
      .addConstructor<void (*)(void)>()
      .addFunction("GetTileX", &TColPacket::GetTileX)
      .addFunction("GetTileY", &TColPacket::GetTileY)
      .addFunction("GetBox", &TColPacket::GetBox)
      .addFunction("GetLayer", &TColPacket::GetLayer)
      .addFunction("GetHmap", &TColPacket::GetHmap)
      .endClass()

      .beginClass<EColPacket>("EColPacket")
      .addConstructor<void (*)(void)>()
      .addFunction("GetName", &EColPacket::GetName)
      .addFunction("GetType", &EColPacket::GetType)
      .addFunction("GetBox", &EColPacket::GetBox)
      .endClass()

      .beginClass<RenderableObject>("RenderableObject")
      .addFunction("GetRotation", &RenderableObject::GetRotation)
      .addFunction("SetRotation", &RenderableObject::SetRotation)

      .addFunction("GetRender", &RenderableObject::GetRender)
      .addFunction("SetRender", &RenderableObject::SetRender)

      .addFunction("SetDepth", &RenderableObject::SetDepth)

      .addFunction("GetScalingX", &RenderableObject::GetScalingX)
      .addFunction("SetScalingX", &RenderableObject::SetScalingX)

      .addFunction("GetScalingY", &RenderableObject::GetScalingY)
      .addFunction("SetScalingY", &RenderableObject::SetScalingY)
      .endClass()

      .beginClass<ComponentSprite>("ComponentSprite")
      .addFunction("AddSprite", &ComponentSprite::AddSprite)
      .addFunction("SetAnimation", &ComponentSprite::SetAnimation)
      .addFunction("SetAnimationSpeed", &ComponentSprite::SetAnimationSpeed)
      .addFunction("AnimationPlayOnce", &ComponentSprite::AnimationPlayOnce)
      .addFunction("GetAnimationSpeed", &ComponentSprite::GetAnimationSpeed)
      .addFunction("DefaultAnimationSpeed",
                   &ComponentSprite::DefaultAnimationSpeed)
      .addFunction("SetImage", &ComponentSprite::SetImageIndex)
      .addFunction("GetImage", &ComponentSprite::GetImageIndex)

      .addFunction("SetRotation", &ComponentSprite::SetRotation)
      .addFunction("SetScaling", &ComponentSprite::SetScaling)
      .addFunction("SetScalingX", &ComponentSprite::SetScalingX)
      .addFunction("SetScalingY", &ComponentSprite::SetScalingY)

      .addFunction("SetOffset", &ComponentSprite::SetOffset)

      .addFunction("RenderSprite", &ComponentSprite::RenderSprite)
      .addFunction("AnimateSprite", &ComponentSprite::AnimateSprite)
      .endClass()

      .beginClass<Shape>("Shape")
      .addData("x", &Shape::x)  // Read-Write
      .addData("y", &Shape::y)  // Read-Write
      .endClass()

      .deriveClass<Rect, Shape>("Rect")
      .addConstructor<void (*)(float, float, float, float)>()

      .addData("w", &Rect::w)  // Read-Write
      .addData("h", &Rect::h)  // Read-Write

      .addFunction("GetTop", &Rect::GetTop)
      .addFunction("GetBottom", &Rect::GetBottom)
      .addFunction("GetLeft", &Rect::GetLeft)
      .addFunction("GetRight", &Rect::GetRight)
      .endClass()

      .beginClass<CollisionBox>("CollisionBox")
      .addFunction("Activate", &CollisionBox::Activate)
      .addFunction("Deactivate", &CollisionBox::Deactivate)
      .addFunction("GetID", &CollisionBox::GetID)
      .addFunction("CheckForEntities", &CollisionBox::CheckForEntities)
      .addFunction("CheckForTiles", &CollisionBox::CheckForTiles)
      .addFunction("SetShape", &CollisionBox::SetShape)
      .addFunction("SetOrder", &CollisionBox::SetOrder)
      .addFunction("RegisterEveryTileCollision",
                   &CollisionBox::RegisterEveryTileCollision)
      .addFunction("RegisterFirstTileCollision",
                   &CollisionBox::RegisterFirstTileCollision)
      .addFunction("CheckForLayer", &CollisionBox::CheckForLayerLuaInterface)
      .addFunction("CheckForLayers", &CollisionBox::CheckForLayersLuaInterface)
      .endClass()

      .beginClass<ComponentCollision>("ComponentCollision")
      .addFunction("SetPrimaryCollisionBox",
                   &ComponentCollision::SetPrimaryCollisionBox)
      .addFunction("AddCollisionBox", &ComponentCollision::AddCollisionBox)
      .endClass()

      .beginClass<ComponentParticle>("ComponentParticle")
      .addFunction("AddParticleCreator", &ComponentParticle::AddParticleCreator)
      .addFunction("DeleteParticleCreators",
                   &ComponentParticle::DeleteParticleCreators)
      .endClass()

      .beginClass<ComponentLight>("ComponentLight")
      .addFunction("CreatePointLight", &ComponentLight::CreatePointLight)
      .endClass()

      .beginClass<Light>("Light")
      .addData("color", &Light::color)
      .addData("pos", &Light::pos)
      .addData("noise", &Light::noise)
      .addData("distance", &Light::distance)
      .addData("render", &Light::render)
      .endClass()

      .deriveClass<PointLight, Light>("PointLight")
      .endClass()

      .beginClass<RSC_Heightmap>("RSC_Heightmap")
      .addFunction("GetHeightMapH", &RSC_Heightmap::GetHeightMapH)
      .addFunction("GetHeightMapV", &RSC_Heightmap::GetHeightMapV)

      .addData("angleH", &RSC_Heightmap::angleH)
      .addData("angleV", &RSC_Heightmap::angleV)
      .endClass()

      .beginClass<Coord2df>("Coord2df")
      .addConstructor<void (*)(void)>()          // Empty Constructor
      .addConstructor<void (*)(float, float)>()  // Constructor
      .addData("x", &Coord2df::x)
      .addData("y", &Coord2df::y)
      .addFunction("Round", &Coord2df::Round)
      .addFunction("Add", &Coord2df::Add)
      .addFunction("Subtract", &Coord2df::Subtract)
      .endClass()

      .beginClass<Vec2>("Vec2")
      .addConstructor<void (*)(void)>()          // Empty Constructor
      .addConstructor<void (*)(float, float)>()  // Constructor
      .addData("x", &Vec2::x)
      .addData("y", &Vec2::y)
      .addFunction("Round", &Vec2::Round)
      .addFunction("Add", &Vec2::Add)
      .addFunction("Subtract", &Vec2::Subtract)
      .endClass()

      .beginClass<Vec3>("Vec3")
      .addConstructor<void (*)(void)>()                 // Empty Constructor
      .addConstructor<void (*)(float, float, float)>()  // Constructor
      .addData("x", &Vec3::x)
      .addData("y", &Vec3::y)
      .addFunction("Round", &Vec3::Round)
      .addFunction("Add", &Vec3::Add)
      .addFunction("Subtract", &Vec3::Subtract)
      .endClass()

      .beginClass<Color4f>("Color4f")
      .addConstructor<void (*)(void)>()  // Empty Constructor
      .addConstructor<void (*)(float, float, float, float)>()  // Constructor
      .addData("r", &Color4f::r)
      .addData("g", &Color4f::g)
      .addData("b", &Color4f::b)
      .addData("a", &Color4f::a)
      .endClass()

      .beginClass<VectorEID>("VectorEID")
      .addFunction("size", &VectorEID::size)
      .addFunction<VectorEID::const_reference (VectorEID::*)(
          VectorEID::size_type) const>("at", &VectorEID::at)
      .addFunction("empty", &VectorEID::empty)
      .endClass()

      .beginClass<VectorString>("VectorString")
      .addFunction("size", &VectorString::size)
      .addFunction<VectorString::const_reference (VectorString::*)(
          VectorString::size_type) const>("at", &VectorString::at)
      .addFunction("empty", &VectorString::empty)
      .endClass()

      /// Maybe add lua friendly wrapper?
      .beginClass<VectorLayer>("VectorLayer")
      .addConstructor<void (*)()>()
      .addFunction("size", &VectorLayer::Size)
      .addFunction("push", &VectorLayer::Push)
      .addFunction("at", &VectorLayer::At)
      .addFunction("empty", &VectorLayer::Empty)
      .endClass()

      .beginClass<BaseComponent>("BaseComponent")
      .addFunction("SetParent", &BaseComponent::SetParentEID)
      .endClass()

      .deriveClass<ComponentPosition, BaseComponent>("ComponentPosition")
      .addFunction("GetPositionLocal", &ComponentPosition::GetPositionLocal)
      .addFunction("GetPositionWorld", &ComponentPosition::GetPositionWorld)
      .addFunction("GetMovement", &ComponentPosition::GetMovement)
      .addFunction("GetAcceleration", &ComponentPosition::GetAcceleration)

      .addFunction("SetPositionLocal", &ComponentPosition::SetPositionLocal)
      .addFunction("SetPositionLocalX", &ComponentPosition::SetPositionLocalX)
      .addFunction("SetPositionLocalY", &ComponentPosition::SetPositionLocalY)

      .addFunction("SetPositionWorld", &ComponentPosition::SetPositionWorld)
      .addFunction("SetPositionWorldX", &ComponentPosition::SetPositionWorldX)
      .addFunction("SetPositionWorldY", &ComponentPosition::SetPositionWorldY)

      .addFunction("SetMovement", &ComponentPosition::SetMovement)
      .addFunction("SetMovementX", &ComponentPosition::SetMovementX)
      .addFunction("SetMovementY", &ComponentPosition::SetMovementY)

      .addFunction("SetAcceleration", &ComponentPosition::SetAcceleration)
      .addFunction("SetAccelerationX", &ComponentPosition::SetAccelerationX)
      .addFunction("SetAccelerationY", &ComponentPosition::SetAccelerationY)

      .addFunction("SetMaxSpeed", &ComponentPosition::SetMaxSpeed)

      .addFunction("IncrementPosition", &ComponentPosition::IncrementMovement)
      .addFunction("IncrementMovement", &ComponentPosition::IncrementMovement)
      .addFunction("IncrementAcceleration",
                   &ComponentPosition::IncrementAcceleration)

      .addFunction("TranslateWorldToLocal",
                   &ComponentPosition::TranslateWorldToLocal)
      .addFunction("TranslateLocalToWorld",
                   &ComponentPosition::TranslateLocalToWorld)
      .endClass()

      .beginClass<ComponentCamera>("ComponentCamera")
      .addFunction("GetViewport", &ComponentCamera::GetViewport)
      .addFunction("SetViewport", &ComponentCamera::SetViewport)
      .endClass()

      .deriveClass<ParticleCreator, RenderableObject>("ParticleCreator")
      .addFunction("SetVelocity", &ParticleCreator::SetVelocity)
      .addFunction("SetAcceleration", &ParticleCreator::SetAcceleration)
      .addFunction("SetPosition", &ParticleCreator::SetPosition)
      .addFunction("SetParticlesPerFrame",
                   &ParticleCreator::SetParticlesPerFrame)
      .addFunction("Start", &ParticleCreator::Start)
      .addFunction("SetColor", &ParticleCreator::SetColor)
      .addFunction("SetScalingX", &ParticleCreator::SetScalingX)
      .addFunction("SetScalingY", &ParticleCreator::SetScalingY)
      .addFunction("SetVertexShaderCode", &ParticleCreator::SetVertexShaderCode)
      .addFunction("SetFragmentShaderCode",
                   &ParticleCreator::SetFragmentShaderCode)
      .addFunction("SetShape", &ParticleCreator::SetShape)
      .addFunction("SetEffect", &ParticleCreator::SetEffect)

      .addFunction("SetSprite", &ParticleCreator::SetSprite)
      .addFunction("SetAnimation", &ParticleCreator::SetAnimation)
      .addFunction("SetAnimationFrame", &ParticleCreator::SetAnimationFrame)
      .addFunction("SetRandomUV", &ParticleCreator::SetRandomUV)
      .addFunction("SetWarpQuads", &ParticleCreator::SetWarpQuads)

      .addFunction("SetUsePoint", &ParticleCreator::SetUsePoint)
      .addFunction("SetPoint", &ParticleCreator::SetPoint)
      .addFunction("SetPointIntensity", &ParticleCreator::SetPointIntensity)
      .endClass()

      .deriveClass<RenderLine, RenderableObject>("RenderLine")
      .addFunction("ChangePosition", &RenderLine::ChangePosition)

      .addFunction("SetColor", &RenderLine::SetColorI)
      .addFunction("SetX1", &RenderLine::SetX1)
      .addFunction("SetY1", &RenderLine::SetY1)
      .addFunction("SetX2", &RenderLine::SetX2)
      .addFunction("SetY2", &RenderLine::SetY2)

      .addFunction("GetX1", &RenderLine::GetX1)
      .addFunction("GetY1", &RenderLine::GetY1)
      .addFunction("GetX2", &RenderLine::GetX2)
      .addFunction("GetY2", &RenderLine::GetY2)
      .endClass()

      .beginClass<RSC_Map>("RSC_Map")
      .addFunction("GetTileLayer", &RSC_Map::GetTileLayer)
      .addFunction("GetSolidTileLayers", &RSC_Map::GetSolidTileLayers)
      .addFunction("GetProperty", &RSC_Map::GetProperty)
      .addFunction("GetWidthTiles", &RSC_Map::GetWidthTiles)
      .addFunction("GetHeightTiles", &RSC_Map::GetHeightTiles)
      .addFunction("GetWidthPixels", &RSC_Map::GetWidthPixels)
      .addFunction("GetHeightPixels", &RSC_Map::GetHeightPixels)

      /// \TODO implement this
      //.addFunction("GetLayersWithPropertyBool", &RSC_Map::GetHeightPixels)

      .endClass()

      .beginClass<GS_Script>("GameState")
      .addFunction("GetStateEID", &GS_Script::GetStateEID)
      .endClass()

      .beginNamespace("ImGui")
      .addFunction("Begin", &ImGui::BeginWrapper)
      .addFunction("BeginFlags", &ImGui::BeginFlags)
      .addFunction("End", &ImGui::End)

      .addFunction("SetWindowPos", &ImGui::SetWindowPosWrapper)
      .addFunction("SetWindowSize", &ImGui::SetWindowSizeWrapper)
      .addFunction("GetWindowSize", &ImGui::GetWindowSizeWrapper)

      .addFunction("SetNextWindowPos", &ImGui::SetNextWindowPosWrapper)
      .addFunction("SetNextWindowFocus", &ImGui::SetNextWindowFocus)
      .addFunction("SetNextWindowSize", &ImGui::SetNextWindowSizeWrapper)
      .addFunction("SetNextWindowPosCenter",
                   &ImGui::SetNextWindowPosCenterWrapper)
      .addFunction("SetNextWindowSizeConstraints",
                   &ImGui::SetNextWindowSizeConstraintsWrapper)

      .addFunction("Text", &ImGui::TextWrapper)
      .addFunction("SliderFloat", &ImGui::SliderFloat)
      .addFunction("Button", &ImGui::ButtonWrapper)

      .addFunction("Sprite", &ImGui::Sprite)
      .addFunction("SpriteButton", &ImGui::SpriteButton)

      .addFunction("ProgressBar", &ImGui::ProgressBarWrapper)

      .addFunction("Separator", &ImGui::Separator)
      .addFunction("SameLine", &ImGui::SameLineWrapper)

      .addFunction("PushStyleColorWindowBG", &ImGui::PushStyleColorWindowBG)
      .addFunction("PushStyleColorButton", &ImGui::PushStyleColorButton)
      .addFunction("PushStyleColorButtonHovered",
                   &ImGui::PushStyleColorButtonHovered)
      .addFunction("PushStyleColorButtonActive",
                   &ImGui::PushStyleColorButtonActive)
      .addFunction("PushStyleColorFrameBG", &ImGui::PushStyleColorFrameBG)
      .addFunction("PushStyleColorFrameBGActive",
                   &ImGui::PushStyleColorFrameBGActive)
      .addFunction("PushStyleColorFrameBGHovered",
                   &ImGui::PushStyleColorFrameBGHovered)
      .addFunction("PushStyleColorText", &ImGui::PushStyleColorText)
      .addFunction("PushStyleColorProgressBarFilled",
                   &ImGui::PushStyleColorPlotHistogram)
      .addFunction("PopStyleColor", &ImGui::PopStyleColor)

      .addFunction("PushFont", &ImGui::PushFontWrapper)
      .addFunction("PopFont", &ImGui::PopFontWrapper)
      .endNamespace()

      .endNamespace();
}

void LuaInterface::SetErrorCallbackFunction(ErrorCallback func) {
  errorCallbackFunction = func;
}

EntityCreationPacket::EntityCreationPacket(std::vector<std::string> scripts,
                                           Coord2df pos, MAP_DEPTH depth,
                                           EID parent, const std::string &name,
                                           luabridge::LuaRef propertyTable)
    : mPropertyTable(propertyTable) {
  mScriptNames = scripts;

  mPos = pos;
  mDepth = depth;
  mParent = parent;
  mEntityName = name;
  mNewEID = 0;
}
