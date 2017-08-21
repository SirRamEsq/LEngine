#ifndef L_STATE_RUNNING
#define L_STATE_RUNNING

#include "../StateManager.h"
#include "../Resources/RSC_Texture.h"
#include "../Resources/RSC_Sprite.h"
#include "../EntityManager.h"

#include "GamePause.h"

#include <GL/gl.h>
#include <GL/glu.h>

class Kernel; //forward declare

class GameRunningState : public GameState{
    public:
        GameRunningState(GameStateManager* gsm);
		~GameRunningState();

        void Init(const RSC_Script* stateScript = NULL);

        void Close();
		void Resume();

        void HandleEvent(const Event* event);
        bool Update();
        void Draw();

    private:
        bool pause;
        int countdown;
        EID ent;
};

#endif
