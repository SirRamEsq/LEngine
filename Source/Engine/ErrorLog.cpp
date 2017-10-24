#include "Defines.h"
#include "Errorlog.h"
#include "SDLInit.h"
#include <exception>

/////////
// Entry//
/////////
Log::Entry::Entry(const std::string &text, Log::SEVERITY s,
                  const std::string &ty, uint32_t time)
    : message(text), severity(s), type(ty), timeStamp(time) {}

std::string Log::Entry::ToString() const {
  std::stringstream ss;
  ss << Log::SEVERITY_STR[severity] << message << std::endl;
  return ss.str();
}

///////
// Log//
///////
std::map<Log::SEVERITY, std::string> Log::SEVERITY_STR = {
    {Log::SEVERITY::FATAL, "[FATAL] "}, {Log::SEVERITY::ERROR, "[ERROR] "},
    {Log::SEVERITY::WARN, "[WARN]  "},  {Log::SEVERITY::INFO, "[INFO]  "},
    {Log::SEVERITY::DEBUG, "[DEBUG] "}, {Log::SEVERITY::TRACE, "[TRACE] "}};
const std::string Log::logPath = "Logs/";
const std::string Log::typeDefault = "DEFAULT";
const std::string Log::fileExtension = ".txt";
Log Log::staticLog = Log();

Log::Log() {
  entryFilter = NULL;
  fileHandle = NULL;
}

Log::~Log() { CloseFileHandle(); }

void Log::SetEntryFilter(fp_EntryFilter filter) { entryFilter = filter; }
void Log::SetEntryFilterFlags(int flags) { filterFlags = flags; }

std::vector<const Log::Entry *> Log::GetEntries() const {
  std::vector<const Entry *> returnEntries;
  int processed = 0;
  // Reverse iterator, start with ewest first
  for (auto i = entries.rbegin(); i != entries.rend(); i++) {
    if (entryFilter != NULL) {
      // ignore entry if returns false
      if (not entryFilter(*i, filterFlags, processed)) {
        continue;
      }
    }

    returnEntries.push_back(&(*i));
    processed++;
  }
  return returnEntries;
}

void Log::Write(const std::string &text, Log::SEVERITY severity,
                const std::string &type) const {
  entries.push_back(Entry(text, severity, type, SDL_GetTicks()));
  if (fileHandle != NULL) {
    std::string str = entries.back().ToString();
    if (PHYSFS_write(fileHandle, str.c_str(), str.size(), 1) == 0) {
      throw LEngineException("PHYSFS Couldn't write");
    }
  }
}

void Log::WriteToFile(const std::string &fileName) {
  CloseFileHandle();
  std::stringstream fPath;
  auto currentDate = GetDateString();
  fPath << "Logs/" << fileName << "_" << currentDate << ".txt";
  fileHandle = PHYSFS_openWrite(fPath.str().c_str());
}

void Log::CloseFileHandle() {
  if (fileHandle != NULL) {
    if (PHYSFS_close(fileHandle) == 0) {
      throw LEngineException("PHYSFS Couldn't Close");
    }
    fileHandle = NULL;
  }
}

std::string Log::GetDateString() {
  time_t rawTime;
  time(&rawTime);
  tm *timeInfo = localtime(&rawTime);

  int bufSize = 100;
  char timeStringBuffer[bufSize];
  // Format 08-16-2017_11-30PM
  strftime(timeStringBuffer, bufSize, "%m-%d-%Y_%I-%M%p", timeInfo);
  return std::string(timeStringBuffer);
}

void Log::WriteEntriesToFile(const std::vector<const Entry *> _entries,
                             const std::string &fileName) {
  // Assumes that PHYSFS_init has been called and that a write directory has
  // been set
  std::stringstream fPath;
  auto currentDate = GetDateString();
  fPath << "Logs/" << fileName << "_" << currentDate << ".txt";

  auto fHandle = PHYSFS_openWrite(fPath.str().c_str());
  for (auto i = _entries.begin(); i != _entries.end(); i++) {
    auto str = (*i)->ToString();
    if (PHYSFS_write(fHandle, str.c_str(), str.size(), 1) == 0) {
      throw LEngineException("PHYSFS Couldn't write");
    }
  }
}
