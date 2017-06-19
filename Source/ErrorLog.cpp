#include "Errorlog.h"
#include "HelperFunctions.h"
#include <exception>

std::map<std::string, std::unique_ptr<std::ofstream> > ErrorLog::errorFiles;
std::map<ErrorLog::SEVERITY, std::string> ErrorLog::SEVERITY_TAGS = {
    {ErrorLog::SEVERITY::FATAL, "[FATAL] "},
    {ErrorLog::SEVERITY::ERROR, "[ERROR] "},
    {ErrorLog::SEVERITY::WARN,  "[WARN]  "},
    {ErrorLog::SEVERITY::INFO,  "[INFO]  "},
    {ErrorLog::SEVERITY::DEBUG, "[DEBUG] "},
    {ErrorLog::SEVERITY::TRACE, "[TRACE] "}
};
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
    auto fpIterator = errorFiles.find(fname);
    std::string fullPath = logPath + fname + fileExtension;
    if(fpIterator == errorFiles.end()){
        std::unique_ptr<std::ofstream> newFP = make_unique<std::ofstream>(fullPath.c_str());

        if(newFP->good() == false){return;}
        if(newFP->is_open() == false){return;}

        *newFP.get() << "Initialized";
        errorFiles[fname] = std::move(newFP);
    }
}

void ErrorLog::CloseFiles(){
    std::ofstream* filePointer=NULL;
    for(auto i=errorFiles.begin(); i!=errorFiles.end(); i++){
        filePointer = i->second.get();
        if(filePointer!=NULL){filePointer->close();}
    }
    errorFiles.clear();
}

std::ofstream* ErrorLog::GetFilePointer(const std::string& fname){
    auto filePointerIt = errorFiles.find(fname);
    if(filePointerIt == errorFiles.end()){
        OpenFile(fname);
        filePointerIt=errorFiles.find(fname);
        if(filePointerIt == errorFiles.end()){
            throw Exception("Couldn't create file for errorlog named " + fname);
        }
    }

    return filePointerIt->second.get();
}

void ErrorLog::WriteToFile(const std::string& text, SEVERITY severity, const std::string& fname){
    std::string newText = SEVERITY_TAGS[severity] + text;
    WriteToFile(newText, fname);
}

void ErrorLog::WriteToFile(const std::string& text, const std::string& fname){
    std::ofstream* filePointer=GetFilePointer(fname);
    if(filePointer==NULL){return;}
    *filePointer << "\n" << text;
    filePointer->flush();
}
