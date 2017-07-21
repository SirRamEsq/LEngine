#include "RSC_Script.h"
#include "../Defines.h"
#include "ResourceLoading.h"

RSC_Script::RSC_Script(std::string sname, const char* dat, unsigned int fsize)
 : scriptName(sname), script(dat,fsize){}

 RSC_Script::~RSC_Script(){
 }

 std::unique_ptr<RSC_Script> RSC_Script::LoadResource(const std::string& fname){
    std::unique_ptr<RSC_Script> script = NULL;
    try{
        std::string fullPath = "Resources/Scripts/"+fname;
        auto data=LoadGenericFile(fullPath);
        if(data.get()->GetData()==NULL){
            return NULL;
        }
        script = make_unique<RSC_Script>(fname, data.get()->GetData(), data.get()->length);
    }
    catch(LEngineFileException e){
        ErrorLog::WriteToFile(e.what(), ErrorLog::GenericLogFile);
    }

    return script;
 }
