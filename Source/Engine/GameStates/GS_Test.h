#ifndef L_SCRIPT_SCRIPT
#define L_SCRIPT_SCRIPT

#include "../EntityManager.h"
#include "../Resources/RSC_Sprite.h"
#include "../Resources/RSC_Texture.h"
#include "../StateManager.h"
#include "GS_Script.h"

struct Assertion {
  Assertion(const std::string &desc, bool p);
  std::string mDescription;
  bool mPass;
};

class GS_Test : public GS_Script {
 public:
  GS_Test(GameStateManager *gsm, const RSC_Script *stateScript);
  ~GS_Test();

  void Init(const RSC_Script *stateScript);

  void HandleEvent(const Event *event);

  /// Will run all test functions and return the assertions made
  std::vector<Assertion> Test();

  /// Empty, no drawing
  void Draw();

  /////////////////////////////////
  // Testing interface functions //
  /////////////////////////////////
  /// Will expose all of the states functionality to lua
  void ExposeTestingInterface(lua_State *state);

  /// Will record a failed assertion
  void Error(const std::string &message);

  /// Will Error if false, Pass if true
  bool REQUIRE_EQUAL(luabridge::LuaRef r1, luabridge::LuaRef r2);
  /// Will Error if false, Pass if true
  bool REQUIRE_NOT_EQUAL(luabridge::LuaRef r1, luabridge::LuaRef r2);

  /// Will call the Kernel Update function
  void KernelUpdate();

  /// Current test index
  int mCurrentTest;
  /// script belonging to this state
  const RSC_Script *mStateScript;
  /// Current script fileName/Path
  std::string mCurrentScriptName;
  std::vector<Assertion> mCurrentTestAssertions;

 private:
  /// Will record a passing assertion
  void Pass(const std::string &message);
  /// Generates a string containing debug info (current lua line#, scriptName,
  /// test#)
  std::string GenerateAssertionString(const std::string &message);
};

#endif
