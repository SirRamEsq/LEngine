#ifndef RY_ERRORLOG
#define RY_ERRORLOG

#include "Exceptions.h"
#include "physfs.h"

#include <fstream>
#include <inttypes.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

class Log {
 public:
  class Exception : public LEngineException {
    using LEngineException::LEngineException;
  };
  enum SEVERITY {
    TRACE = 1,
    DEBUG = 2,
    INFO = 4,
    WARN = 8,
    ERROR = 16,
    FATAL = 32
  };

  struct Entry {
    Entry(const std::string &text, Log::SEVERITY s, const std::string &ty,
          uint32_t time);

    Log::SEVERITY severity;
    std::string type;
    std::string message;
    uint32_t timeStamp;

    std::string ToString() const;
  };

  /// Signature of function used to determine if an entry is filtered or not
  typedef bool (*fp_EntryFilter)(const Entry &entry, int flags, int itemCount);

  Log();
  ~Log();

  /// Set filter to use when calling GetEntries
  void SetEntryFilter(fp_EntryFilter filter);
  /// Set filter flags to use for different severity levels
  void SetEntryFilterFlags(int flags);
  /// Returns all entries. Will use filter set by SetEntryFilter if one was
  /// passed
  std::vector<const Entry *> GetEntries() const;

  /**
   * Will write a new entry to entries
   * \param text Text to write
   * \param severity Severity of message
   * \param type Optional metadata about the Entry's type
   */
  void Write(const std::string &text, SEVERITY severity = SEVERITY::INFO,
             const std::string &type = Log::typeDefault) const;

  /// If called, all entries will henceforth be written to this file
  void WriteToFile(const std::string &fileName, SEVERITY minSeverity = DEBUG);

  /// Default value for Entry::type
  static const std::string typeDefault;
  /// fileExtension used when saving to file
  static const std::string fileExtension;
  /// file path used when saving to file
  static const std::string logPath;

  /// Stores string reprentation of different severity levels
  static std::map<SEVERITY, std::string> SEVERITY_STR;
  /// Will write the entries to a file
  static void WriteEntriesToFile(const std::vector<const Entry *> _entries,
                                 const std::string &fileName);

  static Log staticLog;

  /// Closes currently opened file
  void CloseFileHandle();

  static void LogDirectoryExists();

 protected:
  /// All entries written to this log
  mutable std::vector<Entry> entries;

 private:
  /// Filter used when calling GetEntries
  fp_EntryFilter entryFilter;
  int filterFlags;
  std::ofstream *GetFilePointer(const std::string &fname);

  PHYSFS_File *fileHandle;

  /// Returns the date formated specially for fileNames
  static std::string GetDateString();

  SEVERITY mMinimumWriteSeverity;
};

void LOG_CPP(std::string message, Log::SEVERITY severity, int line,
             std::string file);

// more laconic access
#define LOG_FATAL(stdString) \
  LOG_CPP(stdString, Log::SEVERITY::FATAL, __LINE__, __FILE__);
#define LOG_ERROR(stdString) \
  LOG_CPP(stdString, Log::SEVERITY::ERROR, __LINE__, __FILE__);
#define LOG_WARN(stdString) \
  LOG_CPP(stdString, Log::SEVERITY::WARN, __LINE__, __FILE__);
#define LOG_INFO(stdString) \
  LOG_CPP(stdString, Log::SEVERITY::INFO, __LINE__, __FILE__);
#define LOG_DEBUG(stdString) \
  LOG_CPP(stdString, Log::SEVERITY::DEBUG, __LINE__, __FILE__);
#define LOG_TRACE(stdString) \
  LOG_CPP(stdString, Log::SEVERITY::TRACE, __LINE__, __FILE__);

#endif
