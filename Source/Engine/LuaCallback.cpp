#include "LuaCallback.h"
#include "Errorlog.h"

LuaCallback::LuaCallback(luabridge::LuaRef cb) { SetCallback(cb); }

LuaCallback::LuaCallback() {}

void LuaCallback::SetCallback(luabridge::LuaRef cb) {
  if (!Usable()) {
    mCallback.push_back(cb);
  }
}

bool LuaCallback::Execute() {
  if (Usable()) {
    try {
      mCallback[0]();
    } catch (luabridge::LuaException const &e) {
      std::stringstream ss;
      ss << "Error Running callback function ";
      LOG_ERROR(ss.str());
      return false;
    }
    return true;
  }
  return false;
}

bool LuaCallback::Usable() { return mCallback.size() > 0; }
