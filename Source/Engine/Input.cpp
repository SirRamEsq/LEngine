#include "Input.h"
#include "Kernel.h"
InputManager::InputManager(){
    IniHandler ini;
    std::string iniStr="keyini.txt";
    if(ini.OpenReadFile(iniStr)){
        ini.ReadFileToMap();

        std::map<std::string, std::string>::iterator i;
        i=ini.readItBeg();
        std::string key;
        int val;
        while(i!=ini.readItEnd()){
            key=i->first;
            val=ini.ReadValueNum<int>(key);
            if(val==0){
                val=ini.ReadASCIIValue(key);
            }
            ascii[key]=val;
            asciiREV[val]=key;
            i++;
        }
    }
    ini.CloseReadFile();

    eventDispatcher=NULL;
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
    if(eventDispatcher==NULL){return;}
    SDL_Event event;

    asciiMapREV::iterator keyi;
    KeyMapping::iterator keyMI;

    std::string keyStr;
    bool keyup;

    while(SDL_PollEvent(&event)) {
        if( (keyup = (event.type==SDL_KEYUP) ) or (event.type == SDL_KEYDOWN) ){
            //NOT USING UNICODE!!!
            int asciiValue=event.key.keysym.sym;

            keyi=asciiREV.find(asciiValue);

            if(keyi!=asciiREV.end()){
				if(keyup)   {KeyPress(keyi->second);}
				else        {KeyRelease(keyi->second);}
            }
        }
    }
}

void InputManager::SendEvent(MESSAGE_TYPE message, std::string keyName){
	//TODO change this so that a single event is made and passed around instead of many on the heap
	
	//Stateman is informed first of the event
	eventDispatcher->DispatchEvent(make_unique<Event>(EID_SYSTEM, EID_STATEMAN, message, &keyName));

	//send event to all the entities listening
	auto keyIterator=keyListeners->find(keyName);
	if(keyIterator==keyListeners->end()){return;}
	auto listeners=&keyIterator->second;

	for(auto listener = listeners->begin(); listener!=listeners->end(); listener++){
		eventDispatcher->DispatchEvent(make_unique<Event>(EID_SYSTEM, *listener, message, &keyName));
	}
}

void InputManager::KeyPress(const std::string& keyName){
	SendEvent(MSG_KEYDOWN, keyName);
}

void InputManager::KeyRelease(const std::string& keyName){
	SendEvent(MSG_KEYUP, keyName);
}
