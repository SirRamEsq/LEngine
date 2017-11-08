#ifndef L_SCRIPT_SCRIPT
#define L_SCRIPT_SCRIPT

#include "../EntityManager.h"
#include "../Resources/RSC_Sprite.h"
#include "../Resources/RSC_Texture.h"
#include "../StateManager.h"
#include "GS_Script.h"


class GS_Test : public GS_Script {
 public:
  GS_Test(GameStateManager *gsm);
  ~GS_Test();

  void Init(const RSC_Script *stateScript);

  void HandleEvent(const Event *event);
  bool Update();
  void Draw();

  //Testing interface functions

  ///Will expose all of the states functionality to lua
  void ExposeTestingInterface(lua_State* state);

  void AssertEqual(luabridge::LuaRef r1, luabridge::LuaRef r2);
  void AssertNotEqual(luabridge::LuaRef r1, luabridge::LuaRef r2);


};

#endif
