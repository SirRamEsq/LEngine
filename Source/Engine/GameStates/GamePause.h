#ifndef L_STATE_PAUSE_GAME
#define L_STATE_PAUSE_GAME

#include "../StateManager.h"
#include "../Resources/RSC_Texture.h"
#include "../Resources/RSC_Sprite.h"
#include "../EntityManager.h"
#include <GL/gl.h>
#include <GL/glu.h>

class Kernel; //forward declare

class GamePauseState : public GameState{
    public:
        GamePauseState(GameStateManager* gsm)
            :GameState(gsm){

        }
        void Init();
        void Close();

        void HandleEvent(const Event* event);
        bool Update();
        void Draw();

    private:
        bool exit;
        EID ent;
};

#endif
