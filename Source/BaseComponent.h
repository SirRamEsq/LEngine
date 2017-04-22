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

    public:
        virtual ~BaseComponent();

        BaseComponent(EID id, const std::string& logName) : logFileName(logName){
            mEntityID=id;
        }

        virtual void Update()=0;
        virtual void HandleEvent(const Event* event)=0;

        EID GetEID() const {return mEntityID;}

        const std::string logFileName;

    protected:
        EID mEntityID;
        BaseComponentManager* mManager;
};

class BaseComponentManager{
    public:

        BaseComponentManager(const std::string& logName)
            : logFileName(logName){}

        //for better cache locality, may want to change this from a map of pointers to vector of basecomponents
        //...and use smart pointers, as Valgrind reports a mem leak in the Delete component function
        typedef std::map<EID, BaseComponent*> compMap;
        typedef compMap::iterator compMapIt;

        virtual void AddComponent(EID id)=0; //Create a new component with the specified ID

        void DeleteAllComponents(){
            compMapIt comp=componentList.begin();
            for(;comp!=componentList.end();comp++){
                delete comp->second;
            }
            componentList.clear();
        }

        void DeleteComponent(EID id){
            compMapIt comp=componentList.find(id);
            if(comp!=componentList.end()){
                componentList.erase(id);
                delete comp->second;
            }
        }
        bool HasComponent(EID id){
            if(componentList.find(id)!=componentList.end()){
                return true;
            }
            return false;
        }

        virtual void Update(){
            compMapIt i=componentList.begin();
            for(; i!=componentList.end(); i++){
                i->second->Update();
            }
        }

        virtual void HandleEvent(const Event* event){
            BaseComponent* comp=GetComponent(event->reciever);
            if(comp==NULL){return;}
            comp->HandleEvent(event);
        }

        BaseComponent* GetComponent(EID id){
            compMap::iterator i=componentList.find(id);
            if(i!=componentList.end()){
                return i->second;
            }
            return NULL;
        }

        const std::string logFileName;

    protected:
        std::map<EID, BaseComponent*> componentList;
};

#endif
