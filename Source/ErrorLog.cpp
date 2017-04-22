#include "Errorlog.h"

std::map<std::string, std::ofstream*> ErrorLog::errorFiles;
ErrorLog* ErrorLog::pointertoself=NULL;
ErrorLog::ErrorLog(){}
const std::string ErrorLog::logPath         = "Logs/";
const std::string ErrorLog::GenericLogFile  = "Error";
const std::string ErrorLog::fileExtension   = ".txt" ;

ErrorLog* ErrorLog::Inst(){
    pointertoself=new ErrorLog();
    return pointertoself;
}

void ErrorLog::OpenFile(const std::string& fname){
    std::ofstream* filePointer=errorFiles[fname];
    std::string fullPath = logPath + fname + fileExtension;
    if(filePointer==NULL){
        filePointer= new std::ofstream(fullPath.c_str());
        *filePointer << "Initialized";
        errorFiles[fname]=filePointer;
    }
}

void ErrorLog::CloseFiles(){
    std::ofstream* filePointer=NULL;
    for(auto i=errorFiles.begin(); i!=errorFiles.end(); i++){
        filePointer=i->second;
        if(filePointer!=NULL){filePointer->close();}
    }
    errorFiles.clear();
}

std::ofstream* ErrorLog::GetFilePointer(const std::string& fname){
    std::ofstream* filePointer=errorFiles[fname];
    if(filePointer==NULL){
        OpenFile(fname);
        filePointer=errorFiles[fname];
    }
    return filePointer;
}

void ErrorLog::WriteToFile(const std::string& text, const std::string& fname){
    std::ofstream* filePointer=GetFilePointer(fname);
    if(filePointer==NULL){return;}
    *filePointer << "\n" << text;
    filePointer->flush();
}
