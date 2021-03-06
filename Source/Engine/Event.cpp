#include "Event.h"
#include "EntityManager.h"
#include "Kernel.h"
#include "StateManager.h"

Event::Event(EID send, EID rec, MSG mes, const std::string &desc,
             ExtraDataDefinition *ed)
    : sender(send), reciever(rec), message(mes), description(desc) {
  if (ed != NULL) {
    ed->SetExtraData(this);
  }
}

EventDispatcher::EventDispatcher() {
  gameStateManager = NULL;
  gameStateEntityManager = NULL;
}

void EventDispatcher::SetDependencies(GameStateManager *gs, EntityManager *em) {
  gameStateManager = gs;
  gameStateEntityManager = em;

  if (gameStateManager == NULL) {
    LOG_DEBUG("GameStateManager is NULL in the EventDispatcher");
  }
  if (gameStateEntityManager == NULL) {
    LOG_DEBUG("EntityManager is NULL in the EventDispatcher");
  }
};

void EventDispatcher::DispatchEvent(const Event &event) {
  EID id = event.reciever;

  if (id == EID_ALLOBJS) {
    BroadcastEvent(event);
  } else if (id == EID_STATEMAN) {
    gameStateManager->HandleEvent(&event);
  } else {
    gameStateEntityManager->DispatchEvent(&event);
  }
}

void EventDispatcher::DispatchEvent(const Event &event,
                                    const std::vector<EID> *entities) {
  for (auto i = entities->begin(); i != entities->end(); i++) {
    event.reciever = (*i);
    DispatchEvent(event);
  }
}

void EventDispatcher::DispatchEvent(const Event &event,
                                    const std::set<EID> *entities) {
  for (auto i = entities->begin(); i != entities->end(); i++) {
    event.reciever = (*i);
    DispatchEvent(event);
  }
}

void EventDispatcher::BroadcastEvent(const Event &event) {
  gameStateManager->HandleEvent(&event);
  gameStateEntityManager->BroadcastEvent(&event);
}
