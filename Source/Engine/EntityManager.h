#ifndef L_ENTITY_MAN
#define L_ENTITY_MAN

#include "BaseComponent.h"
#include "Exceptions.h"
#include "Defines.h"
#include "Event.h"

#include <vector>
#include <map>


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
            INPUT       = 10,
            PARTICLE    = 20,
            POSITION    = 30,
            COLLISION   = 40,
            SCRIPT      = 50,
            CAMERA      = 60,
            LIGHT       = 70,
            SPRITE      = 80
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
		EID NewEntity(const std::string& name="");
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
		 * Deletes all EIDs between first reserved EID and EID_MIN
		 */
		void ClearAllReservedEntities();
		
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

		/**
		 * Sets the parent of each component of an entity
		 */
		void SetParent(EID child, EID parent);

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
