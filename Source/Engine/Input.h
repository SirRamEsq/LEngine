#ifndef L_SDL_INPUT
#define L_SDL_INPUT

#include "SDL2/SDL.h"
#include "EntityManager.h"
#include "Event.h"

#include "IniHandler.h"

#include <map>
#include <set>

class Kernel; //forward declare

/**
 * Handles input from SDL, sends events to StateManager and Entities inside the state
 */
class InputManager{
    typedef std::map<std::string, int> asciiMap;
    typedef std::map<int, std::string>  asciiMapREV;

    public:
        typedef std::set<EID> EntitySet;
        typedef std::map<std::string, EntitySet> KeyMapping;

        InputManager();
        ~InputManager();

		/**
		 * Process all events from SDL, sends events specified by the key mapping to the dispatcher.
		 * StateManager is notified first of input, then each registered entity
		 */
        void HandleInput();

		/**
		 * Send key presses defined by the keymapping to the event dispatcher
		 * if no key mapping is specified, a new one is created
		 * \return the keymapping that will be used
		 */
        std::shared_ptr<KeyMapping> SetEventDispatcher(EventDispatcher* e, std::shared_ptr<KeyMapping>* mapping=NULL);

    private:
        asciiMap    ascii;
        asciiMapREV asciiREV;

        EventDispatcher* eventDispatcher;
        std::shared_ptr<KeyMapping> keyListeners;
};

#endif
