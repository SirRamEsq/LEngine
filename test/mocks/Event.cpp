#include "Event.h"
#include "Kernel.h"

EventDispatcher::EventDispatcher(void* gs, void* em){
    //gameStateManager  = gs;
    //gameStateEntityManager  = em;

    //if(gameStateManager==NULL){ErrorLog::WriteToFile("GameStateManager is NULL in the EventDispatcher", ErrorLog::GenericLogFile);}
    //if(gameStateEntityManager==NULL){ErrorLog::WriteToFile("EntityManager is NULL in the EventDispatcher", ErrorLog::GenericLogFile);}
};

void EventDispatcher::DispatchEvent(std::unique_ptr<Event> event){
    events.push_back(std::move(event));
    /*EID id=event->reciever;
    if     (id==EID_STATEMAN){gameStateManager      ->HandleEvent   (event.get());}
    //else if(id==EID_ALLOBJS) {gameStateEntityManager->BroadcastEvent(event);}
    else                     {gameStateEntityManager->DispatchEvent (event.get());}*/
}

void EventDispatcher::DispatchImmediateEvent(std::unique_ptr<Event> event){
    /*EID id=event->reciever;
    if     (id==EID_STATEMAN){gameStateManager      ->HandleEvent   (event.get());}
    //else if(id==EID_ALLOBJS) {gameStateEntityManager->BroadcastEvent(event);}
    else                     {gameStateEntityManager->DispatchEvent (event.get());}*/
}
