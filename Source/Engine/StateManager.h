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

		RSC_Map* GetCurrentMap(){return mCurrentMap.get();}


		RenderManager				renderMan;
		///Stores passed arguments till the start of the next frame, then loads the map
		void SetMapNextFrame(const RSC_Map* m, unsigned int entranceID);

	protected:
		virtual void Init(const RSC_Script* stateScript = NULL)=0;
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
		bool SetCurrentMap(const RSC_Map* m, unsigned int entranceID);

		const RSC_Map* nextMap;
		unsigned int nextMapEntrance;
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
		std::unique_ptr<RSC_Map> mCurrentMap;
		std::vector<std::unique_ptr<RenderableObjectWorld> > mCurrentMapTileLayers;
};

/**
 * Manages and owns all GameStates
 */
class GameStateManager{
	friend Kernel;
	friend EventDispatcher;
	friend EntityManager;
	friend GameState;

	public:
		GameStateManager();
		GameState* GetCurrentState(){return mCurrentState;}

		/**
		 * Prepares to push State Next Frame
		 * Sets internal nextFrameState and nextFrameScript
		 * this class assumes ownership of the state
		 * Can optionally pass a script to be run
		 */
		void PushState(std::unique_ptr<GameState> state, const RSC_Script* script = NULL);

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
		InputManager input;

		///Sets 'remapKey' and remaps next frame
		void RemapKey(const std::string& key);

	protected:
		void Close();

		void HandleEvent(const Event* event);
		bool Update();
		void Draw();

		void DrawPreviousState(GameState* gs);
		void PushNextState();

	private:
		GameState* GetPreviousState(GameState* gs);
		GameState* mCurrentState;
		///State to push next frame
		std::unique_ptr<GameState> nextFrameState;
		const RSC_Script* nextFrameStateScript;
		std::vector<std::unique_ptr<GameState> > mGameStates;

		///Next key to remap
		std::string remapKey;

		/*
		 * Input Manager which is used to pass key mappings to each individual state
		 * Handles Input and uses the current state's key mapping to send events
		 */
};

#endif
