#ifndef L_SDL_INPUT
#define L_SDL_INPUT

#include "SDL2/SDL.h"
#include "EntityManager.h"
#include "Event.h"

#include "IniHandler.h"

#include <map>
#include <vector>

class Kernel; //forward declare

class InputManager{
    typedef std::map<std::string, int> asciiMap;
    typedef std::map<int, std::string>  asciiMapREV;
    typedef std::vector<EID> entityVec;
    typedef std::map<std::string, entityVec> keyMapping;
    public:
        InputManager();
        ~InputManager();

        void ListenForInput(std::string input, EID id); //Set a certain entity to listen for certain input
        void ClearAllListeners();

        void HandleInput();

    private:
        keyMapping      keyListeners;

        asciiMap    ascii;
        asciiMapREV asciiREV;
};

#endif
