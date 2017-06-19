#include "LScript.h"

LScript::LScript(std::string sname, const char* dat, unsigned int fsize)
 : scriptName(sname), script(dat,fsize){}

 LScript::~LScript(){
 }

 std::unique_ptr<LScript> LScript::LoadResource(const std::string& fname){
    LScript* script = NULL;
    try{
        std::string fullPath = "Resources/Scripts/"+fname;
        auto data=LoadGenericFile(fullPath);
        if(data.get()->GetData()==NULL){
            return NULL;
        }
        script = new LScript(fname, data.get()->GetData(), data.get()->length);
    }
    catch(LEngineFileException e){
        ErrorLog::WriteToFile(e.what(), ErrorLog::GenericLogFile);
    }

    return script;
 }
