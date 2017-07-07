#include "IniHandler.h"
/*

INI File must take the form
key = value;
key=value;
key =value;
key= value;

and end with a ';'
*/
IniHandler::IniHandler(){}

IniHandler::~IniHandler(){
    CloseReadFile();
    CloseWriteFile();
}

bool IniHandler::OpenReadFile(std::string& fname){
    if(iniRead.is_open()){CloseReadFile();}
    iniRead.open(fname.c_str());
    if(!iniRead.good()){
        ErrorLog::WriteToFile("IniHandler::OpenReadFile file isn't good", ErrorLog::GenericLogFile);
        return false;
    }
    return true;
}

bool IniHandler::OpenWriteFile(std::string& fname){
    if(iniWrite.is_open()){CloseWriteFile();}
    iniWrite.open(fname.c_str());
    if(!iniWrite.good()){
        ErrorLog::WriteToFile("IniHandler::OpenWriteFile file isn't good", ErrorLog::GenericLogFile);
        return false;
    }
    return true;
}

void IniHandler::CloseReadFile(){
    ClearReadMap();
    if(iniRead.is_open()){iniRead.close();}
}

void IniHandler::CloseWriteFile(){
    ClearWriteMap();
    if(iniWrite.is_open()){iniWrite.close();}
}

void IniHandler::ClearReadMap(){
    readMap.clear();
}

void IniHandler::ClearWriteMap(){
    writeMap.clear();
}


bool IniHandler::ReadFileToMap(){
    int SIZEOFBUFFER=255;
    int CstringSize;
    char stringBuffer[SIZEOFBUFFER];
    char* tok;
    char* key;
    char* value;
    std::string skey, svalue;

    while(!iniRead.eof()){
        iniRead.getline(stringBuffer, SIZEOFBUFFER, ';\n');
        CstringSize=strlen(stringBuffer);
        //If the string contains no characters or only one blank character, continue
        if(CstringSize<2){break;}
        tok=strtok (stringBuffer," ;=\n"); //space, semicolon, newline, and equal are delimiters
        key=tok;
        tok=strtok (NULL," ;=\n");
        value=tok;

        if(key==NULL){continue;}
        if(tok==NULL){continue;}

        skey=key;
        if(strcmp(value, "true")==0){svalue="1";} //if value = the string 'true' set the string to 1 instead
        if(strcmp(value, "false")==0){svalue="0";} //if value = the string 'false' set the string to 0 instead
        else{svalue=value;}

        readMap[skey]=svalue;
        //ErrorLog::WriteToFile(skey, " " + svalue);
    }
}


