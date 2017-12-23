#ifndef LENGINE_LUA_CALLBACK
#define LENGINE_LUA_CALLBACK

extern "C" {
#include "lua5.2/lauxlib.h"
#include "lua5.2/lua.h"
#include "lua5.2/lualib.h"
}
#include <LuaBridge.h>
#include <vector>

/**
 * Purpose of this class is to delay initialization of a lua ref
 */
class LuaCallback {
 public:
  LuaCallback(luabridge::LuaRef cb);
  LuaCallback();

  void SetCallback(luabridge::LuaRef cb);

  // return true on success
  bool Execute();
  bool Usable();

 private:
  // using std::Vector to delay initialization of the callback
  // only one callback is actually intended to be used
  std::vector<luabridge::LuaRef> mCallback;
};

#endif
