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

        void Init(const RSC_Script* stateScript);
        void Close();

		///If the script entity wants to pop this state, it must delete itself
        void HandleEvent(const Event* event);
        bool Update();
        void Draw();

	private:
		bool quit;
		ComponentScript* entityScript;
};

#endif