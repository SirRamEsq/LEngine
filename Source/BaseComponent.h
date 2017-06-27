#ifndef L_BASECOMP
#define L_BASECOMP

#include "Defines.h"
#include "Event.h"

#include <map>

class BaseComponentManager;
class ComponentPositionManager;
class BaseComponent{
    friend class BaseComponentManager;
    friend class ComponentPositionManager;

    //callback function called on event
    typedef void (*EventFunction)(const Event* event);

    public:
        virtual ~BaseComponent();

        BaseComponent(EID id, const std::string& logName) : logFileName(logName){
            mEntityID=id;
        }

        virtual void Update()=0;
        virtual void HandleEvent(const Event* event);

        EID GetEID() const {return mEntityID;}

        const std::string logFileName;
        void SetEventCallbackFunction(EventFunction f);

    protected:
        EID mEntityID;
        BaseComponentManager* mManager;

    private:
        EventFunction eventCallback;
};

class BaseComponentManager{
    public:
        BaseComponentManager(const std::string& logName)
            : logFileName(logName){}

        //for better cache locality, may want to change this from a map of pointers to vector of basecomponents
        //...and use smart pointers, as Valgrind reports a mem leak in the Delete component function
        typedef std::map<EID, BaseComponent*> compMap;
        typedef compMap::iterator compMapIt;

        virtual void    AddComponent        (EID id)=0; //Create a new component with the specified ID
        virtual void    AddComponent        (std::unique_ptr<BaseComponent> comp); //Add a loaded component (good for mocking)
        void            DeleteAllComponents ();
        void            DeleteComponent     (EID id);
        bool            HasComponent        (EID id);
        BaseComponent*  GetComponent        (EID id);
        int             GetComponentCount   ();

        virtual void Update();
        virtual void HandleEvent(const Event* event);

        const std::string logFileName;

    protected:
        std::map<EID, BaseComponent*> componentList;
};

#endif
