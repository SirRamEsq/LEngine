#include "GS_Test.h"
//#include "../../Testing/catch.hpp"
#include "../Engine/Kernel.h"

std::string StringRepresentationOfRef(const luabridge::LuaRef &r1) {
  std::stringstream ss;
  switch (r1.type()) {
    case LUA_TNIL:
      ss << "NIL";
      break;

    case LUA_TBOOLEAN:
      ss << "boolean: " << (r1.cast<bool>() ? "true" : "false");
      break;

    case LUA_TNUMBER:
      ss << "number: " << r1.cast<lua_Number>();
      break;

    case LUA_TSTRING:
      ss << "string: \"" << r1.cast<std::string>() << '"';
      break;

    case LUA_TTABLE:
      ss << r1.tostring();
      break;

    case LUA_TFUNCTION:
      ss << r1.tostring();
      break;

    case LUA_TTHREAD:
      ss << r1.tostring();
      break;

    case LUA_TUSERDATA:
      ss << r1.tostring();
      break;

    case LUA_TLIGHTUSERDATA:
      ss << r1.tostring();
      break;

    default:
      ss << "???";
  }

  return ss.str();
}

Assertion::Assertion(const std::string &desc, bool p)
    : mDescription(desc), mPass(p) {}

GS_Test::GS_Test(GameStateManager *gsm, const RSC_Script *stateScript)
    : GS_Script(gsm) {
  quit = false;
  if (stateScript == NULL) {
    throw LEngineException("GS_Test::Init, stateScript is NULL");
    quit = true;
    return;
  }
  entityScript = NULL;

  mStateScript = stateScript;
  ExposeTestingInterface(luaInterface.GetState());
}

GS_Test::~GS_Test() {
  mCurrentTest = 0;
  mCurrentScriptName = "";
}

void GS_Test::Init(const RSC_Script *ignore) {
  std::string scriptName = "STATE";
  std::string scriptType = "Common/__BaseState.lua";
  MAP_DEPTH depth = 0;
  EID parent = 0;
  EID eid = EID_RESERVED_STATE_ENTITY;
  quit = false;

  auto baseScript = K_ScriptMan.GetLoadItem(scriptType, scriptType);
  // REQUIRE (baseScript != NULL);

  comScriptMan.AddComponent(eid);
  std::vector<const RSC_Script *> scripts;
  scripts.push_back(baseScript);
  scripts.push_back(mStateScript);
  luaInterface.RunScript(eid, scripts, NULL, NULL);
  entityScript = comScriptMan.GetComponent(eid);
  // REQUIRE(entityScript != NULL);
}

void GS_Test::HandleEvent(const Event *event) {
  if (event->message == Event::MSG::KEYDOWN) {
    std::string inputKey = event->description;

    if (inputKey == "use") {
      quit = true;
    }
  }

  else if (event->message == Event::MSG::ENTITY_DELETED) {
    if (event->sender == EID_RESERVED_STATE_ENTITY) {
      entityScript = NULL;
      quit = true;
    }
  }
}

void GS_Test::KernelUpdate() { Kernel::Update(); }

/*
GS_TEST will simply use the inhereted function
bool GS_Test::Update() {
  if (nextMap != NULL) {
    auto tempMap = nextMap;
    auto tempEntrance = nextMapEntrance;
    SetCurrentMap(tempMap, tempEntrance);
    nextMap = NULL;
    nextMapEntrance = 0;
  }
  UpdateComponentManagers();
  return !quit;
}
*/

std::vector<Assertion> GS_Test::Test() {
  mCurrentTestAssertions.clear();
  if (entityScript == NULL) {
    std::stringstream errorMessage;
    errorMessage << "Couldn't load test script";
    Assertion fail(errorMessage.str(), false);
    mCurrentTestAssertions.push_back(fail);
    return mCurrentTestAssertions;
  }
  try {
    luabridge::LuaRef testTable = entityScript->GetScriptPointer()["TESTS"];
    // REQUIRE(testTable.isNil() == false);
    mCurrentScriptName = entityScript->scriptName;
    auto setupFunction = entityScript->GetFunction("Setup");
    auto teardownFunction = entityScript->GetFunction("Teardown");
    /*
    Table must be declared like
        table = {val1, val2, etc...}
    */
    for (int i = 0; i < testTable.length(); i++) {
      // note the i + 1 here, it's because arrays in Lua start with 1
      mCurrentTest = i;
      luabridge::LuaRef testFunction = testTable[i + 1];
      // REQUIRE(testFunction.isNil() == false);

      if (setupFunction.isNil() == false) {
        setupFunction(this);
      }

      testFunction(this);

      if (teardownFunction.isNil() == false) {
        teardownFunction(this);
      }
    }

  } catch (const luabridge::LuaException &e) {
    mCurrentTestAssertions.push_back(Assertion(e.what(), false));
  }
  return mCurrentTestAssertions;
}

void GS_Test::Draw() {}

// Testing
void GS_Test::ExposeTestingInterface(lua_State *state) {
  using namespace luabridge;
  getGlobalNamespace(state)            // global namespace to lua
      .beginNamespace("CPP")           //'CPP' table
      .beginClass<GS_Test>("GS_Test")  // define class object
      .addFunction("Error", &GS_Test::Error)
      .addFunction("Update", &GS_Test::KernelUpdate)
      .addFunction("REQUIRE_EQUAL", &GS_Test::REQUIRE_EQUAL)
      .addFunction("REQUIRE_NOT_EQUAL", &GS_Test::REQUIRE_NOT_EQUAL)
      .addFunction("REQUIRE_NO_THROW", &GS_Test::REQUIRE_NO_THROW)
      .addFunction("REQUIRE_THROW", &GS_Test::REQUIRE_THROW)
      .endClass()
      .endNamespace();
}

bool GS_Test::REQUIRE_EQUAL(luabridge::LuaRef r1, luabridge::LuaRef r2) {
  std::string strR1 = StringRepresentationOfRef(r1);
  std::string strR2 = StringRepresentationOfRef(r2);

  std::stringstream ss;
  ss << std::endl << "    " << strR1 << " == " << strR2 << std::endl;

  if (r1 == r2) {
    Pass(ss.str());
    return true;
  }

  Error(ss.str());
  return false;
}

bool GS_Test::REQUIRE_NOT_EQUAL(luabridge::LuaRef r1, luabridge::LuaRef r2) {
  std::string strR1 = StringRepresentationOfRef(r1);
  std::string strR2 = StringRepresentationOfRef(r2);

  std::stringstream ss;
  ss << std::endl << "    " << strR1 << " != " << strR2 << std::endl;

  if (r1 == r2) {
    Error(ss.str());
    return false;
  }
  Pass(ss.str());
  return true;
}

bool GS_Test::REQUIRE_THROW(luabridge::LuaRef callback) {
  try {
    if (callback.isFunction()) {
      callback();
    } else {
      Error("Callback is not function!");
      return false;
    }
  } catch (const luabridge::LuaException &e) {
    Pass("Callback successfully threw a LuaException");
    return true;
  }
  Error("Callback Did not throw an expected LuaException");
  return false;
}

bool GS_Test::REQUIRE_NO_THROW(luabridge::LuaRef callback) {
  try {
    if (callback.isFunction()) {
      callback();
    } else {
      Error("Callback is not function!");
      return false;
    }
  } catch (const luabridge::LuaException &e) {
    std::stringstream ss;
    ss << "Callback threw an unexpected LuaException\n" << e.what();
    Error(ss.str());
    return false;
  }
  Pass("Callback Did not throw a LuaException");
  return true;
}

std::string GS_Test::GenerateAssertionString(const std::string &message) {
  lua_Debug ar;
  auto L = luaInterface.GetState();
  lua_getstack(L, 1, &ar);
  lua_getinfo(L, "nSl", &ar);
  int line = ar.currentline;
  std::stringstream ss;
  ss << std::endl
     << "Lua Testing Assertion " << std::endl
     << "ScriptName: " << mCurrentScriptName << std::endl
     << "TestFunction#: " << mCurrentTest << std::endl
     << "Line: " << line << std::endl
     << "Message: '" << message << "'" << std::endl;

  return ss.str();
}

void GS_Test::Error(const std::string &message) {
  auto debugInfo = GenerateAssertionString(message);
  mCurrentTestAssertions.push_back(Assertion(debugInfo, false));
}

void GS_Test::Pass(const std::string &message) {
  auto debugInfo = GenerateAssertionString(message);
  mCurrentTestAssertions.push_back(Assertion(debugInfo, true));
}
