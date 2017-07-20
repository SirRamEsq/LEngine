#ifndef L_EVENTS
#define L_EVENTS

#include "Defines.h"

#include <memory>
#include <vector>

class GameStateManager;
class EntityManager;
class EventDispatcher;

class Event{
	friend EventDispatcher;

	public:
		//using the visitor pattern to set and get the void pointer
		class ExtraDataDefinition{
			public:
				//To dervice from this class correctly...
				//write a constructor that grabs the data structure you want to set to extradata
				//store the datastructure in a member variable
				//assign the member variable to the event->extradata in the SetExtraData function
				virtual void SetExtraData(Event* event)=0;
				//Should write a static getter to convert the void pointer to the desired type
		};

		enum MSG{
			COLLISION_ENTITY = 0,
			COLLISION_TILE,

			KEYDOWN,
			KEYUP,
			LISTEN_TO_KEY,

			MAP_PUSH,
			MAP_POP,
			MAP_CHANGE,
			LUA_EVENT,
			ENTITY_DELETED
		};

		Event(EID send, EID rec, MSG mes, const std::string& desc, ExtraDataDefinition* ed = NULL);
		
		const EID sender;
		mutable EID reciever;
		const MSG message;
		const std::string description;

    	const void* extradata;

};

// \TODO instead of passing a unique ptr to an event, simply pass an Event by value
// \TODO should be able to pass a list of entities to send events to
/**
 * Dispatches Events in the correct order and in the most efficent way possible
 */
class EventDispatcher{
    public:
        EventDispatcher();
        void SetDependencies(GameStateManager* gs, EntityManager* em);
        void DispatchEvent(const Event& event, const std::vector<EID>* entities = NULL);

    private:
        //Send to all entities listening for this specific event
        void BroadcastEvent (const Event& event);
		void SendEventToMutlipleEntities (const Event& event, const std::vector<EID>& entities);

        GameStateManager* gameStateManager;
        EntityManager*    gameStateEntityManager;
};

#endif
