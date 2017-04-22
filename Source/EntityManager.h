#ifndef L_ENTITY_MAN
#define L_ENTITY_MAN

#include <vector>
#include <map>

#include "Defines.h"
#include "Event.h"

class EntityManager{
    public:
        EntityManager(){entityNumber=EID_MIN;mFlagDeleteAll=false;}

        //Can optionally pass in a name to associate with the EID
        EID NewEntity(const std::string& name=""); //return an EID that is not in use
        EID GetEIDFromName(const std::string& name) const;

        void DeleteEntity   (EID id); //Remove all components with EID id from their managers
        void DispatchEvent  (const Event* event);

        void Cleanup();

        void ClearAllEntities();

    protected:
        void ClearNameMappings();

    private:
        EID entityNumber;
        bool mFlagDeleteAll;

        std::vector<EID> deadEntities;

        //Map of names to EIDS and an inverse lookup map (this is fine because both the names and EIDs are unique
        std::map<std::string, EID> nameToEID;
        std::map<EID, std::string> EIDToName;
};

#endif
