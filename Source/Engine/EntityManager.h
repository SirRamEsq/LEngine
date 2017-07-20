#ifndef L_ENTITY_MAN
#define L_ENTITY_MAN

#include "BaseComponent.h"
#include "Exceptions.h"
#include "Defines.h"
#include "Event.h"

#include <vector>
#include <map>

#define EID_SYSTEM      0 //EID that refers to the system
#define EID_STATEMAN    1 //EID that refers to the statemanager
#define EID_MAPSCRIPT   2 //EID for the script belonging to the whole map
#define EID_ALLOBJS     3 //EID that broadcasts to all components
#define EID_MIN         10 //First EID that can be used by components
/// \TODO Add components into EntityManager; They're already added in and sorted by order. If you move them from StateManager to here, then that same order can be used to update the component Managers. Also adds a generic way to add more component managers should the need arise and adjust the order they update in

/**
 * Manages Entity IDs (EIDs), Component Managers, and ensures that deleted entities are fully deleted at the appropriate time
 */
class EntityManager{
    public:
        class Exception : public LEngineException{using LEngineException::LEngineException;};
		/**
		 * Defines an order to update the component managers in an order that should be acceptable to most GameStates
		 */
        enum DEFAULT_UPDATE_ORDER{
            PARTICLE    = 10,
            COLLISION   = 20,
            INPUT       = 30,
            POSITION    = 40,
            SCRIPT      = 50,
            SPRITE      = 60,
            CAMERA      = 70,
            LIGHT       = 80
        };

        EntityManager(){
			entityNumber=EID_MIN;
			mFlagDeleteAll=false;
			entityCount = 0;
		}

        /**
		 * Creates a New Entity
		 * Can optionally pass in a name to associate with the EID
		 * \return The new Entity ID value
		 */
        EID NewEntity(const std::string& name=""); //return an EID that is not in use
		/**
		 * Can lookup an eid by a string name, if the entity was given one upon creation
		 */
        EID GetEIDFromName(const std::string& name) const;

		/**
		 * Schedules all components with the given entity to be deleted from their managers
		 */
        void DeleteEntity   (EID id); 
		/**
		 * Calls 'DeleteEntity' on all active eids
		 * \see DeleteEntity(EID id)
		 */
        void ClearAllEntities();
		
		/**
		 * Dispatches an Event to all registered Component Managers
		 */
        void DispatchEvent  (const Event* event);

		/**
		 * Broadcasts an event to all registered Component Managers
		 */
		void BroadcastEvent (const Event* event);

		/**
		 * Actually deletes all entities scheduled to be deleted.
		 */
        void Cleanup();

		/**
		 * Can Register a new Component Manager to recieve events, have entities added to/deleted from, etc...
		 */
        void RegisterComponentManager(BaseComponentManager* manager, int order);

		/**
		 * Returns a count of currently active entities
		 */
		unsigned int GetEntityCount();

    protected:
        void ClearNameMappings();

    private:
        EID entityNumber;
		unsigned int entityCount;
        bool mFlagDeleteAll;

        std::vector<EID> deadEntities;

        //Map of names to EIDS and an inverse lookup map (this is fine because both the names and EIDs are unique
        std::map<std::string, EID> nameToEID;
        std::map<EID, std::string> EIDToName;

        //components sorted by priority in ascending order
        std::map<int, BaseComponentManager*, std::less<int>> componentsRegistered;
};

#endif
