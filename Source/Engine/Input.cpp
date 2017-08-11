#include "Input.h"
#include <algorithm>


const std::string InputManager::defaultKeyMappingIniFileName = "keyini.txt";

InputManager::InputManager()
	: mouseX(0), mouseY(0), mouseWheel(0), 
	mouseButtonLeft(false), mouseButtonRight(0), mouseButtonMiddle(0){
	ReadKeyIniFile();
    eventDispatcher=NULL;
}

void InputManager::ReadKeyIniFile(){
	ascii.clear();
	asciiREV.clear();
    if(keyMappingIni.OpenReadFile("keyini.txt")){
        keyMappingIni.ReadFileToMap();

        std::map<std::string, std::string>::iterator i;
        i=keyMappingIni.readItBeg();
        std::string key;
        int value;
        while(i!=keyMappingIni.readItEnd()){
            key=i->first;
            value=keyMappingIni.ReadValueNum<int>(key);
            if(value==0){
                value=keyMappingIni.ReadASCIIValue(key);
            }
            ascii[key]=value;
            asciiREV[value]=key;
            i++;
        }
    }
    keyMappingIni.CloseReadFile();
	keyMappingIni.CopyReadMapToWriteMap();
}

InputManager::~InputManager(){

}

void InputManager::SimulateKeyPress(const std::string& keyName){
	KeyPress(keyName);	
}

void InputManager::SimulateKeyRelease(const std::string& keyName){
	KeyRelease(keyName);
}

std::shared_ptr<InputManager::KeyMapping> InputManager::SetEventDispatcher(EventDispatcher* e, std::shared_ptr<InputManager::KeyMapping>* mapping){
    //will set key mapping to one specified, or generate a new one if none is specified
    eventDispatcher = e;
    if(mapping == NULL){
        keyListeners = std::shared_ptr<KeyMapping>(new KeyMapping());
    }
    else{
        keyListeners = *mapping;
    }

    //will return data structure to allow caller to directly set key listeners
    return keyListeners;
}

void InputManager::HandleInput(){
	//Reset mousewheel every frame
	mouseWheel = 0.0f;
	if(remapKey != ""){
		WriteMapSetKeyToNextInput(remapKey);
		ReadKeyIniFile();
		remapKey = "";
	}

    if(eventDispatcher==NULL){return;}
    SDL_Event event;

    asciiMapREV::iterator keyi;
    KeyMapping::iterator keyMI;

    std::string keyStr;
    bool keyup;

    while(SDL_PollEvent(&event)) {
        if( (keyup = (event.type==SDL_KEYUP) ) or (event.type == SDL_KEYDOWN) ){
            int asciiValue=event.key.keysym.sym;

            keyi=asciiREV.find(asciiValue);

            if(keyi!=asciiREV.end()){
				if(!keyup)   {KeyPress(keyi->second);}
				else        {KeyRelease(keyi->second);}
            }
        }

		else if (event.type == SDL_MOUSEWHEEL){
			mouseWheel = event.wheel.y;
		}

    	else if (event.type == SDL_TEXTINPUT){
            sdlTextInput = (event.text.text);
        }
    }

	//MouseX and MouseY are set the mouse's x,y coordinates relative to the origin of the SDLWindow
    auto mouseMask = SDL_GetMouseState(&mouseX, &mouseY);
	mouseButtonLeft = ( (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0);
	mouseButtonRight = ( (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0);
	mouseButtonMiddle = ( (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0);
}

void InputManager::SendEvent(Event::MSG message, std::string keyName){
	Event event(EID_SYSTEM, EID_STATEMAN, message, keyName);

	//Stateman is informed first of the event
	eventDispatcher->DispatchEvent(event);

	//send event to all the entities listening
	auto keyIterator=keyListeners->find(keyName);
	if(keyIterator==keyListeners->end()){
		return;
	}

	auto listeners=&keyIterator->second;
	eventDispatcher->DispatchEvent(event, listeners);
}

void InputManager::KeyPress(const std::string& keyName){
	SendEvent(Event::MSG::KEYDOWN, keyName);
}

void InputManager::KeyRelease(const std::string& keyName){
	SendEvent(Event::MSG::KEYUP, keyName);
}

bool InputManager::WriteMapSetKeyToNextInput(const std::string& key){
	if(key == ""){return false;}

	std::string value = "";
	//time in miliseconds
	auto timer = SDL_GetTicks();
	//Ten seconds
	Uint32 timeLimit = 10 * 1000;
	timer += timeLimit;

    SDL_Event event;
    while( (SDL_PollEvent(&event)) or (timer > SDL_GetTicks()) ) {
        if( (event.type == SDL_KEYDOWN) ){
            SDL_Keycode keyCode = event.key.keysym.sym;
			value = SDL_GetKeyName(keyCode);
			break;
        }
    }
	if(value == ""){return false;}

	//lowerCase
	std::transform(value.begin(), value.end(), value.begin(), ::tolower);

	keyMappingIni.WriteString(key, value);
	OverwriteKeyIni();
	return true;
}

void InputManager::OverwriteKeyIni(){
	keyMappingIni.OpenWriteFile("keyini.txt");
	keyMappingIni.WriteMapToFile();
	keyMappingIni.CloseWriteFile();
}

Coord2df InputManager::GetMousePosition(){
	return Coord2df(mouseX, mouseY);
}

bool InputManager::GetMouseButtonLeft(){
	return mouseButtonLeft;
}

bool InputManager::GetMouseButtonRight(){
	return mouseButtonRight;
}

bool InputManager::GetMouseButtonMiddle(){
	return mouseButtonMiddle;
}

float InputManager::GetMouseWheel(){
	return mouseWheel;
}


void InputManager::RemapKey(const std::string keyName){
	remapKey = keyName;
}
