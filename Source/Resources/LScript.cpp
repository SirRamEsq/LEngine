#include "LScript.h"

LScript::LScript(std::string sname, const char* dat, unsigned int fsize)
 : scriptName(sname), script(dat,fsize){}

 LScript::~LScript(){
 }
