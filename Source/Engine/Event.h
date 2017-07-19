#ifndef L_EVENTS
#define L_EVENTS

#include "Defines.h"

#include <memory>
#include <vector>

//EventManager MUST be instantiated after EntityManager and StateManager

class GameStateManager;
class EntityManager;

struct Event{
    const EID sender;
    const EID reciever;
    const MESSAGE_TYPE message;
    std::string eventDescription;
    //this should probably be changed to a smart pointer, to ensure the data is still valid when the event is read
    const void* extradata;

    Event(EID send, EID rec, MESSAGE_TYPE mes, const void* dat=NULL)
        :sender(send), reciever(rec), message(mes), extradata(dat){

        eventDescription="event";
    }
};

//This struct will automatically be populated by and attactched to an event by the engine when a lua entity sends an event
struct EntityEvent : public Event{
    std::string name;
    std::string type;
    EID         senderID;
};

// \TODO instead of passing a unique ptr to an event, simply pass an Event by value
/**
 * Creates and Dispatches Events in the correct order and in the most efficent way possible
 */
class EventDispatcher{
    public:
        EventDispatcher();
        void SetDependencies(GameStateManager* gs, EntityManager* em);
        void DispatchEvent(std::unique_ptr<Event> event);
        //Immediate Events typcially slower but are good if you need a response from another object
        void DispatchImmediateEvent(std::unique_ptr<Event> event);

        //Send to all entities listening for this specific event
        void BroadcastEvent (std::unique_ptr<Event> event);

    private:
        GameStateManager* gameStateManager;
        EntityManager*    gameStateEntityManager;
};

#endif
