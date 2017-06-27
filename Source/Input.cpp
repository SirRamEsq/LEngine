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

InputManager::~InputManager(){}

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
    MESSAGE_TYPE msg= MSG_KEYDOWN;

    //This eats up loads of unnecessary CPU
    //if(SDL_WaitEvent(&event)){

    while(SDL_PollEvent(&event)) {
        if( (keyup = (event.type==SDL_KEYUP) ) or (event.type == SDL_KEYDOWN) ){
            //NOT USING UNICODE!!!
            int asciiValue=event.key.keysym.sym;

            if(keyup)   {msg=MSG_KEYUP;}
            else        {msg=MSG_KEYDOWN;}
            keyi=asciiREV.find(asciiValue);

            if(keyi!=asciiREV.end()){
                keyStr=keyi->second;
                //Stateman is informed first of the event
                eventDispatcher->DispatchEvent( std::unique_ptr<Event>(new Event(EID_SYSTEM, EID_STATEMAN, msg, &keyStr)) );

                //send event to all the entities listening
                keyMI=keyListeners->find(keyStr);
                if(keyMI==keyListeners->end()){continue;}
                auto vec=&keyMI->second;

                for(auto veci = vec->begin(); veci!=vec->end(); veci++){
                    eventDispatcher->DispatchEvent( std::unique_ptr<Event>(new Event(EID_SYSTEM, *veci, msg, &keyStr)) );
                }
            }
        }
    }
}
