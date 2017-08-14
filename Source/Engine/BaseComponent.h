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

		BaseComponent* parent;

    private:
		//Pointer to component's parent, should be same type of component as derived class BaseComponent* parent;
		///Can be used to handle an event (optional)
        EventFunction eventCallback;
};

class BaseComponentManager{
    public:
        BaseComponentManager(const std::string& logName, EventDispatcher* e)
            : logFileName(logName), eventDispatcher(e){}

        /// \TODO use smart pointers
        typedef std::map<EID, BaseComponent*> compMap;
        typedef compMap::iterator compMapIt;

	   	///Create a new component with the specified ID
        virtual void    AddComponent        (EID id, EID parent=0)=0;
 		///Add a loaded component (good for mocking)
		virtual void    AddComponent        (std::unique_ptr<BaseComponent> comp);

		void            DeleteAllComponents ();
        void            DeleteComponent     (EID id);

        bool            HasComponent        (EID id);
        BaseComponent*  GetComponent        (EID id);
        int             GetComponentCount   ();

		/**
		 * Updates all components once
		 * Sets and unsets the component's 'updatedThisFrame' boolean
		 **/
        void Update();
		/**
		 * Handles recieved events
		 * If an entity is deleted that acts as a parent for another entity
		 * the componentManager should set that entity's parent to NULL
		 * The component that has the deleted entity as a parent may also access the old parent
		 * and change parents before the manager forces the parent to be null
		 */
        virtual void HandleEvent(const Event* event);
        virtual void BroadcastEvent(const Event* event);
		/**
		 * Sets parent of a component
		 * Both parent and child must exist as a component of this manager
		 * if parent is 0, then parent of the child is set to NULL
		 */
		virtual void SetParent(EID child, EID parent);

        const std::string logFileName;

    protected:
		/**
		 * Recursive function, updates parents before children
		 * sets 'updatedThisFrame' on components after being updated
		 * otherwise the entities are ordered and updated by EID
		 */
		virtual void UpdateComponent(BaseComponent* child);

        std::map<EID, BaseComponent*> componentList;
        EventDispatcher* eventDispatcher;
};

#endif
