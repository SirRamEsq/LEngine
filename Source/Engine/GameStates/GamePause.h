#ifndef L_STATE_PAUSE_GAME
#define L_STATE_PAUSE_GAME

#include "../EntityManager.h"
#include "../Resources/RSC_Sprite.h"
#include "../Resources/RSC_Texture.h"
#include "../StateManager.h"
#include <GL/gl.h>
#include <GL/glu.h>

class Kernel;  // forward declare

class GamePauseState : public GameState {
 public:
  GamePauseState(GameStateManager *gsm);

  ~GamePauseState();
  void Init(const RSC_Script *stateScript = NULL);

  void Close();
  void Resume();

  void HandleEvent(const Event *event);
  bool Update();
  void Draw();

 private:
  bool exit;
  EID ent;
};

#endif
