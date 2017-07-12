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
#include "Resources/RSC_Map.h"
#include "LuaInterface.h"


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
		GameState(GameStateManager* gsm);

		//Virtual destructor; enables derived classes to be fully deleted from a base GameState pointer
		virtual ~GameState(){
		}

		I_RSC_Map* GetCurrentMap(){return mCurrentMap.get();}


		RenderManager				renderMan;
		bool SetCurrentMap(const I_RSC_Map* m, unsigned int entranceID);

	protected:
		virtual void Init()=0;
		virtual void Close()=0;

		virtual void HandleEvent(const Event* event)=0;
		virtual bool Update()=0;
		virtual void Draw()=0;
		virtual void SetDependencies();
		void UpdateComponentManagers();

		LuaInterface				luaInterface;
		EventDispatcher				eventDispatcher;
		EntityManager				entityMan;

		ComponentPositionManager	comPosMan;
		ComponentInputManager		comInputMan;
		ComponentScriptManager		comScriptMan;
		ComponentSpriteManager		comSpriteMan;
		ComponentCollisionManager	comCollisionMan;
		ComponentParticleManager	comParticleMan;
		ComponentCameraManager		comCameraMan;
		ComponentLightManager		comLightMan;

		GameStateManager*			gameStateManager;

		std::shared_ptr<InputManager::KeyMapping> input;

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
		std::unique_ptr<I_RSC_Map> mCurrentMap;
		std::vector<std::unique_ptr<RenderableObjectWorld> > mCurrentMapTileLayers;
};

/**
 * Manages and owns all GameStates
 */
class GameStateManager{
	friend Kernel;
	friend EventDispatcher;
	friend GameState;

	public:
		GameStateManager();
		GameState* GetCurrentState(){return mCurrentState;}

		/**
		 * Push and Initialize state
		 * this class assumes ownership of the state
		 */
		void PushState(std::unique_ptr<GameState> state);
		/**
		 * State is popped and deleted
		 */
		void PopState();

		int stackSize(){
			return mGameStates.size();
		}

		bool IsEmpty(){
			return mGameStates.empty();
		}
		void UpdateCurrentState();

	protected:
		void Close();

		void HandleEvent(const Event* event);
		bool Update();
		void Draw();

		void DrawPreviousState(GameState* gs);

	private:
		GameState* GetPreviousState(GameState* gs);
		GameState* mCurrentState;
		std::vector<std::unique_ptr<GameState> > mGameStates;

		/*
		 * Input Manager which is used to pass key mappings to each individual state
		 * Handles Input and uses the current state's key mapping to send events
		 */
		InputManager input;
};

#endif
