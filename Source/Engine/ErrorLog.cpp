#include "Errorlog.h"
#include "Defines.h"
#include "SDLInit.h"
#include <exception>

/////////
//Entry//
/////////
Log::Entry::Entry(const std::string& text, Log::SEVERITY s, const std::string& ty, uint32_t time)
	: message(text), severity(s), type(ty), timeStamp(time){

}

std::string Log::Entry::ToString() const{
	std::stringstream ss;
	ss << "[" <<  Log::SEVERITY_STR[severity] << "] "
		<< message;
	return ss.str();
}

///////
//Log//
///////
std::map<Log::SEVERITY, std::string> Log::SEVERITY_STR = {
    {Log::SEVERITY::FATAL, "[FATAL] "},
    {Log::SEVERITY::ERROR, "[ERROR] "},
    {Log::SEVERITY::WARN,  "[WARN]  "},
    {Log::SEVERITY::INFO,  "[INFO]  "},
    {Log::SEVERITY::DEBUG, "[DEBUG] "},
    {Log::SEVERITY::TRACE, "[TRACE] "}
};
const std::string Log::logPath       = "Logs/";
const std::string Log::typeDefault   = "DEFAULT";
const std::string Log::fileExtension = ".txt" ;

Log::Log(){
	entryFilter = NULL;
}

Log::~Log(){

}

void Log::SetEntryFilter(fp_EntryFilter filter){
	entryFilter = filter;
}

std::vector<const Log::Entry*> Log::GetEntries() const{
	std::vector<const Entry*> returnEntries;	
	for(auto i = entries.begin(); i != entries.end(); i++){
		if(entryFilter != NULL){
			//ignore entry if returns false
			if(not entryFilter(*i)){
				continue;
			}
		}

		returnEntries.push_back( &(*i) );
	}
	return returnEntries;
}

void Log::Write(const std::string& text, Log::SEVERITY severity, const std::string& type) const{
	entries.push_back(Entry(text,severity,type,SDL_GetTicks()));
}

void Log::WriteToFile(const std::vector<const Entry*> _entries, const std::string& fileName) const{

}

/*
std::map<std::string, std::unique_ptr<std::ofstream> > ErrorLog::errorFiles;
std::map<Log::SEVERITY, std::string> Log::SEVERITY_TAGS = {
    {Log::SEVERITY::FATAL, "[FATAL] "},
    {Log::SEVERITY::ERROR, "[ERROR] "},
    {Log::SEVERITY::WARN,  "[WARN]  "},
    {Log::SEVERITY::INFO,  "[INFO]  "},
    {Log::SEVERITY::DEBUG, "[DEBUG] "},
    {Log::SEVERITY::TRACE, "[TRACE] "}
};
ErrorLog* ErrorLog::pointertoself=NULL;
ErrorLog::ErrorLog(){}
const std::string ErrorLog::logPath         = "Logs/";
const std::string Log::typeDefault  = "Error";
const std::string ErrorLog::fileExtension   = ".txt" ;
bool ErrorLog::noThrow   = true ;

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
        if(noThrow){
            return NULL;
        }
        if(filePointerIt == errorFiles.end()){
            throw Exception("Couldn't create file for errorlog named " + fname);
        }
    }

    return filePointerIt->second.get();
}

void K_Log.Write(const std::string& text, SEVERITY severity, const std::string& fname){
    std::string newText = SEVERITY_TAGS[severity] + text;
    WriteToFile(newText, fname);
}

void K_Log.Write(const std::string& text, const std::string& fname){
    std::ofstream* filePointer=GetFilePointer(fname);
    if(filePointer==NULL){return;}
    *filePointer << "\n" << text;
    filePointer->flush();
}
*/
