#ifndef L_STATE_START_GAME
#define L_STATE_START_GAME

#include "../EntityManager.h"
#include "../Resources/RSC_Sprite.h"
#include "../Resources/RSC_Texture.h"
#include "../StateManager.h"

#include "GamePause.h"

#include <GL/gl.h>
#include <GL/glu.h>

class Kernel;  // forward declare

class GameStartState : public GameState {
 public:
  GameStartState(GameStateManager *gsm);
  ~GameStartState();

  void Init(const RSC_Script *stateScript = NULL);

  void Close();
  void Resume();

  void HandleEvent(const Event *event);
  bool Update();
  void Draw();

};

#endif
