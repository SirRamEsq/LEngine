#ifndef LENGINE_BASE_COMP_MAN
#define LENGINE_BASE_COMP_MAN

#include "BaseComponent.h"

<template T>
class BaseComponentManager{
	//Template should have BaseComponent as Base
	static_assert(std::is_base_of<BaseComponent, T>::value, "T must derive from BaseComponent");

    public:
		BaseComponentManager();

	   	///Create a new component with the specified ID
        virtual void    AddComponent        (EID id, EID parent=0);
 		///Add a loaded component (good for mocking)
		virtual void    AddComponent        (std::unique_ptr<T> comp);

		void      	DeleteAllComponents ();
        void      	DeleteComponent     (EID id);

        bool     	HasComponent        (EID id);
        T*  		GetComponent        (EID id);
        int 		GetComponentCount   ();

		/**
		 * Updates all active components once
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

    protected:
		/**
		 * Recursive function, updates parents before children
		 * sets 'updatedThisFrame' on components after being updated
		 * otherwise the entities are ordered and updated by EID
		 */
		void UpdateComponent(T* child);

		///All Components that this Manager owns
        std::map<EID, std::unique_ptr<T> > componentList;
		///Components that are active and will be updated
        std::map<EID, T*> activeComponentList;
        EventDispatcher* eventDispatcher;
};

//Include definitions
#include "BaseComponentManagerDef.cpp"

#endif
