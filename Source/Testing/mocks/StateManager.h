#ifndef L_MOCK_GS_MAN
#define L_MOCK_GS_MAN
#include "../../Engine/StateManager.h"
#include "../../Engine/Event.h"

class GameStateManager_Mock : public GameStateManager{
	friend Kernel;
	friend EventDispatcher;
	friend EntityManager;
	friend GameState;

	public:
		GameStateManager_Mock(InputManager* input) : GameStateManager(input){

		}
		virtual GameState* GetCurrentState(){

		}

		virtual void PushState(const std::shared_ptr<GameState>& state, const RSC_Script* script = NULL){

		}

		virtual void SwapState(const std::shared_ptr<GameState>& state, const RSC_Script* script = NULL){

		}

		virtual void PopState(){

		}
		virtual int StackSize(){

		}
		virtual bool IsEmpty(){

		}
		InputManager* inputManager;

	protected:
		virtual void Close(){

		}

		virtual void HandleEvent(const Event* event){

		}
		virtual bool Update(){

		}
		virtual void Draw(){

		}

		virtual void DrawPreviousState(GameState* gs){

		}
		virtual void PushNextState(){

		}
		virtual void PopTopState(){

		}
};

#endif
