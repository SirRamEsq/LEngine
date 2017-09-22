#ifndef LENGINE_BASE_COMP
#define LENGINE_BASE_COMP

#include "Defines.h"
#include "Event.h"

#include <map>
#include <memory>

class BaseComponentManager;
class ComponentPositionManager;
class BaseComponent{
    friend class BaseComponentManager;
    friend class ComponentPositionManager;

    //callback function called on event
    typedef void (*EventFunction)(const Event* event);

    public:
        virtual ~BaseComponent();

        BaseComponent(EID id, const std::string& logName, BaseComponent* parent=NULL); 

        virtual void Update()=0;
        virtual void HandleEvent(const Event* event);

        EID GetEID() const {return mEntityID;}

        const std::string logFileName;
        void SetEventCallbackFunction(EventFunction f);

		virtual void SetParent(BaseComponent* p);
		///wrapper around 'SetParent', used mainly for scripting interface
		void SetParentEID(EID p);
		virtual BaseComponent* GetParent();

		//Used by ComponentManager to determine if this component was already updated this frame
		bool updatedThisFrame;
    protected:
        EID mEntityID;
        BaseComponentManager* mManager;

		///Pointer to component's parent, should be same type of component as derived class BaseComponent* parent;
		BaseComponent* parent;

    private:
		///Can be used to handle an event (optional)
        EventFunction eventCallback;
};

#endif
