#include "../../Engine/StateManager.h"

class GameStateMock : public GameState{
	public:

		GameStateMock(GameStateManager* gm);
		~GameStateMock();

		void Init();
		void Close();
		void HandleEvent(const Event*);
		bool Update();
		void Draw();

		//Expose otherwise private members
		EntityManager* GetEntityMan();
		LuaInterface* GetLuaInterface();
		ComponentScriptManager* GetComponentManagerScript();
		ComponentPositionManager* GetComponentManagerPosition();
		EventDispatcher* GetEventDispatcher();

		int drawCount;
		int updateCount;
		bool close;
		bool init;
};
