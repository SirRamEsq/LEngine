#ifndef L_STATE_SCRIPT
#define L_STATE_SCRIPT

#include "../StateManager.h"
#include "../Resources/RSC_Texture.h"
#include "../Resources/RSC_Sprite.h"
#include "../EntityManager.h"

#include <GL/gl.h>
#include <GL/glu.h>

class Kernel; //forward declare

class GS_Script : public GameState{
    public:
       	GS_Script(GameStateManager* gsm);
		~GS_Script();

        void Init(const RSC_Script* stateScript);

        void Close();
		void Resume();

        void HandleEvent(const Event* event);
        bool Update();
        void Draw();

	////////////////////
	//SCRIPT INTERFACE//
	////////////////////
		EID GetStateEID() const ;

		GS_Script* PushState(const std::string& scriptName);
		GS_Script* PushedStateGet(int index);
		void PushedStateDelete(int index);
		void PushedStateDeleteAll();

	private:
		bool quit;
		ComponentScript* entityScript;

		///Stores previously pushed but now popped states
		std::vector< std::shared_ptr<GS_Script> > pushedStates;
};

#endif
