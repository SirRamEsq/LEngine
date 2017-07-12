#ifndef L_ENTITY_MAN
#define L_ENTITY_MAN

#include "BaseComponent.h"
#include "Exceptions.h"
#include "Defines.h"
#include "Event.h"

#include <vector>
#include <map>

class EntityManager{
    public:
        class Exception : public LEngineException{using LEngineException::LEngineException;};
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

        //Can optionally pass in a name to associate with the EID
        EID NewEntity(const std::string& name=""); //return an EID that is not in use
        EID GetEIDFromName(const std::string& name) const;

        void DeleteEntity   (EID id); //Remove all components with EID id from their managers
        void DispatchEvent  (const Event* event);

        void Cleanup();

        void ClearAllEntities();

        void RegisterComponentManager(BaseComponentManager* manager, int order);

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
