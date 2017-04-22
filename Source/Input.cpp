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
}

InputManager::~InputManager(){}

void InputManager::ListenForInput(std::string input, EID id){
    /*keyMapping::iterator i;
    i=keyListeners.find(input);
    if(i==keyListeners.end()){return;}

    i->second.push_back(id);*/
    keyListeners[input].push_back(id);
}

void InputManager::ClearAllListeners(){
    keyMapping::iterator i=keyListeners.begin();
    for(; i!=keyListeners.end(); i++){
        i->second.clear();
    }
}

void InputManager::HandleInput(){
    SDL_Event event;

    entityVec::iterator veci;
    std::vector<EID>* vec;
    asciiMapREV::iterator keyi;
    keyMapping::iterator keyMI;

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
                K_EventMan.DispatchEvent( std::unique_ptr<Event>(new Event(EID_SYSTEM, EID_STATEMAN, msg, &keyStr)) );

                //send event to all the entities listening
                keyMI=keyListeners.find(keyStr);
                if(keyMI==keyListeners.end()){continue;}
                vec=&keyMI->second;
                veci=vec->begin();

                while(veci!=vec->end()){
                    K_EventMan.DispatchEvent( std::unique_ptr<Event>(new Event(EID_SYSTEM, *veci, msg, &keyStr)) );
                    veci++;
                }
            }
        }
    }
}
