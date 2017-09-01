#include "IniHandler.h"
#include "Kernel.h"

IniHandler::IniHandler(){}

IniHandler::~IniHandler(){
    CloseReadFile();
    CloseWriteFile();
}

bool IniHandler::OpenReadFile(const std::string& fname){
    if(iniRead.is_open()){CloseReadFile();}
    iniRead.open(fname.c_str());
    if(!iniRead.good()){
        K_Log.Write("IniHandler::OpenReadFile file isn't good", Log::SEVERITY::ERROR);
        return false;
    }
    return true;
}

bool IniHandler::OpenWriteFile(const std::string& fname){
    if(iniWrite.is_open()){CloseWriteFile();}
    iniWrite.open(fname.c_str());
    if(!iniWrite.good()){
        K_Log.Write("IniHandler::OpenWriteFile file isn't good", Log::SEVERITY::ERROR);
        return false;
    }
    return true;
}

void IniHandler::CloseReadFile(){
    if(iniRead.is_open()){iniRead.close();}
}

void IniHandler::CloseWriteFile(){
    if(iniWrite.is_open()){iniWrite.close();}
}

void IniHandler::ClearReadMap(){
    readMap.clear();
}

void IniHandler::ClearWriteMap(){
    writeMap.clear();
}


bool IniHandler::ReadFileToMap(){
    if(!iniRead.is_open()){
		throw LEngineException("IniHandler readfile is not open!");
	}
    if(!iniRead.good()){
		throw LEngineException("IniHandler readfile is not good!");
	}
    int SIZEOFBUFFER=255;
    int CstringSize;
    char stringBuffer[SIZEOFBUFFER];
    char* tok;
    char* key;
    char* value;
    std::string keyString, valueString;

    while(!iniRead.eof()){
        iniRead.getline(stringBuffer, SIZEOFBUFFER, ';\n');
        CstringSize=strlen(stringBuffer);
        //If the string contains no characters or only one blank character, continue
        if(CstringSize < 2){break;}
        tok=strtok (stringBuffer," ;=\n"); //space, semicolon, newline, and equal are delimiters
        key=tok;
        tok=strtok (NULL," ;=\n");
        value=tok;

        if(key==NULL){continue;}
        if(tok==NULL){continue;}

        keyString=key;
		//if value = the string 'true' set the string to 1 instead
        if(strcmp(value, "true")==0){valueString="1";}
		//if value = the string 'false' set the string to 0 instead
		else if(strcmp(value, "false")==0){valueString="0";} 
        else{valueString=value;}

        readMap[keyString]=valueString;
    }
}

bool IniHandler::WriteMapToFile(){
    if(!iniWrite.is_open()){
		throw LEngineException("IniHandler writeFile is not open!");
	}
    if(!iniWrite.good()){
		throw LEngineException("IniHandler writeFile is not good!");
	}

	for(auto i = writeMap.begin(); i != writeMap.end(); i++){
		std::stringstream output;
		output << i->first << " = " << i->second << ";\n";
		iniWrite.write(output.str().c_str(), output.str().size());
	}
}

bool IniHandler::WriteString(const std::string& key, const std::string& value){
	if( (key != "") and (value != "") ){
		writeMap[key] = value;
		return true;
	}
	return false;
}

void IniHandler::CopyReadMapToWriteMap(){
	for(auto i = readMap.begin(); i != readMap.end(); i++){
		writeMap[i->first] = i->second;
	}
}
