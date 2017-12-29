#ifndef L_MOCK_GS_MAN
#define L_MOCK_GS_MAN
#include "../../Engine/Event.h"
#include "../../Engine/StateManager.h"

class GameStateManager_Mock : public GameStateManager {
  friend Kernel;
  friend EventDispatcher;
  friend EntityManager;
  friend GameState;

 public:
  GameStateManager_Mock(InputManager *input) : GameStateManager(input) {}
  virtual GameState *GetCurrentState() {return NULL;}

  virtual void PushState(const std::shared_ptr<GameState> &state,
                         const RSC_Script *script = NULL) {}

  virtual void SwapState(const std::shared_ptr<GameState> &state,
                         const RSC_Script *script = NULL) {}

  virtual void PopState() {return;}
  virtual int StackSize() {return 0;}
  virtual bool IsEmpty() {return true;}
  InputManager *inputManager;

 protected:
  virtual void Close() {}

  virtual void HandleEvent(const Event *event) {}
  virtual bool Update() {return true;}
  virtual void Draw() {}

  virtual void DrawPreviousState(GameState *gs) {}
  virtual void PushNextState() {}
  virtual void PopTopState() {}
};

#endif
