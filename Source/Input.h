#ifndef L_SDL_INPUT
#define L_SDL_INPUT

#include "SDL2/SDL.h"
#include "EntityManager.h"
#include "Event.h"

#include "IniHandler.h"

#include <map>
#include <set>

class Kernel; //forward declare

class InputManager{
    typedef std::map<std::string, int> asciiMap;
    typedef std::map<int, std::string>  asciiMapREV;

    public:
        typedef std::set<EID> EntitySet;
        typedef std::map<std::string, EntitySet> KeyMapping;

        InputManager();
        ~InputManager();

        void HandleInput();

        std::shared_ptr<KeyMapping> SetEventDispatcher(EventDispatcher* e, std::shared_ptr<KeyMapping>* mapping=NULL);

    private:
        //KeyMapping      keyListeners;

        asciiMap    ascii;
        asciiMapREV asciiREV;
        EventDispatcher* eventDispatcher;

        std::shared_ptr<KeyMapping> keyListeners;
};

#endif
