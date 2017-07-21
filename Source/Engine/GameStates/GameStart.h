#ifndef L_STATE_START_GAME
#define L_STATE_START_GAME

#include "../StateManager.h"
#include "../Resources/RSC_Texture.h"
#include "../Resources/RSC_Sprite.h"
#include "../EntityManager.h"

#include "GamePause.h"

#include <GL/gl.h>
#include <GL/glu.h>

class Kernel; //forward declare

class GameStartState : public GameState{
    public:
        GameStartState(GameStateManager* gsm)
            : GameState(gsm){

        }

        void Init();
        void Close();

        void HandleEvent(const Event* event);
        bool Update();
        void Draw();

    private:
        bool pause;
        int countdown;
        EID ent;
};

#endif
