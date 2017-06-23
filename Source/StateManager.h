#ifndef L_STATE_MAN
#define L_STATE_MAN

#include <stack>
#include <memory>

#include "Components/CompPosition.h"
#include "Components/CompScript.h"
#include "Components/CompInput.h"
#include "Components/CompSprite.h"
#include "Components/CompCollision.h"
#include "Components/CompParticle.h"
#include "Components/CompParticle.h"
#include "Components/CompCamera.h"
#include "Components/CompLight.h"

#include "Event.h"
#include "Resources/LMap.h"
#include "LuaInterface.h"

//State manager owns all states given to it

class LuaInterfaceB;

class GameStateManager;
class GameState{
    friend GameStateManager;
    friend EventDispatcher;
    friend LuaInterface;
    friend EntityManager;
    friend RenderManager;

    friend ComponentPositionManager;
    friend ComponentInputManager;
    friend ComponentScriptManager;
    friend ComponentSpriteManager;
    friend ComponentCollisionManager;
    friend ComponentParticleManager;
    friend ComponentCameraManager;
    friend ComponentLightManager;

    public:
        GameState(GameStateManager* gsm)
            :gameStateManager(gsm), luaInterface(this, SCREEN_W, SCREEN_H, CAMERA_W, CAMERA_H),
             comScriptMan(luaInterface.GetState(), &luaInterface){
            mCurrentMap=NULL;
        }

        //Virtual destructor; enables derived classes to be fully deleted from a base GameState pointer
        virtual ~GameState(){
        }

        LMap* GetCurrentMap(){return mCurrentMap.get();}


        RenderManager               renderMan;
        bool SetCurrentMap(const LMap* m, unsigned int entranceID);

    protected:
        virtual void Init()=0;
        virtual void Close()=0;

        virtual void HandleEvent(const Event* event)=0;
        virtual bool Update()=0;
        virtual void Draw()=0;
        void UpdateComponentManagers();

        LuaInterface                luaInterface;

        ComponentPositionManager    comPosMan;
        ComponentInputManager       comInputMan;
        ComponentScriptManager      comScriptMan;
        ComponentSpriteManager      comSpriteMan;
        ComponentCollisionManager   comCollisionMan;
        ComponentParticleManager    comParticleMan;
        ComponentCameraManager      comCameraMan;
        ComponentLightManager       comLightMan;

        GameStateManager*           gameStateManager;

        void DrawPreviousState();

    private:
        void SetMapHandleRenderableLayers(const std::map <MAP_DEPTH, TiledLayerGeneric*>& layers);
        //returns a data structure mapping tiled EIDS to engine EIDS
        std::map<EID,EID> SetMapCreateEntitiesFromLayers(const std::vector<std::unique_ptr<TiledObjectLayer> >& layers);
        std::vector<EID> SetMapGetEntitiesUsingEntrances(const std::vector<std::unique_ptr<TiledObjectLayer> >& layers);
        void SetMapLinkEntities(
                const std::vector<std::unique_ptr<TiledObjectLayer> >& layers,
                const std::map<EID,EID>& tiledIDtoEntityID,
                const std::vector<EID>& objectsUsingEntrance
        );


        //is copy of what is stored in resource manager
        std::unique_ptr<LMap> mCurrentMap;
        std::vector<std::unique_ptr<RenderableObjectWorld> > mCurrentMapTileLayers;
};

class GameStateManager{
    friend Kernel;
    friend EventDispatcher;
    friend GameState;

    public:
        GameState* GetCurrentState(){return mCurrentState;}

        //Pass smart pointer by value; this class assumes ownership of the pointer
        void PushState(std::unique_ptr<GameState> state);
        void PopState();

        int stackSize(){
            return mGameStates.size();
        }

        bool IsEmpty(){
            return mGameStates.empty();
        }

    protected:
        void Init();
        void Close();

        void HandleEvent(const Event* event);
        bool Update();
        void Draw();

        void UpdateCurrentState();

        void DrawPreviousState(GameState* gs);

    private:
        GameState* GetPreviousState(GameState* gs);
        GameState* mCurrentState;
        std::vector<std::unique_ptr<GameState> > mGameStates;
};

#endif
