#include "Event.h"
#include "Kernel.h"
#include "StateManager.h"
#include "EntityManager.h"

#ifndef TEST_Event_MOCK


EventDispatcher::EventDispatcher(){
    gameStateManager        = NULL;
    gameStateEntityManager  = NULL;
}

void EventDispatcher::SetDependencies(GameStateManager* gs, EntityManager* em){
    gameStateManager        = gs;
    gameStateEntityManager  = em;

    if(gameStateManager==NULL){ErrorLog::WriteToFile("GameStateManager is NULL in the EventDispatcher", ErrorLog::GenericLogFile);}
    if(gameStateEntityManager==NULL){ErrorLog::WriteToFile("EntityManager is NULL in the EventDispatcher", ErrorLog::GenericLogFile);}
};

void EventDispatcher::DispatchEvent(std::unique_ptr<Event> event){
    EID id=event->reciever;
    if     (id==EID_STATEMAN){gameStateManager      ->HandleEvent   (event.get());}
    //else if(id==EID_ALLOBJS) {gameStateEntityManager->BroadcastEvent(event);}
    else                     {gameStateEntityManager->DispatchEvent (event.get());}
}

void EventDispatcher::DispatchImmediateEvent(std::unique_ptr<Event> event){
    EID id=event->reciever;
    if     (id==EID_STATEMAN){gameStateManager      ->HandleEvent   (event.get());}
    //else if(id==EID_ALLOBJS) {gameStateEntityManager->BroadcastEvent(event);}
    else                     {gameStateEntityManager->DispatchEvent (event.get());}
}

#endif
