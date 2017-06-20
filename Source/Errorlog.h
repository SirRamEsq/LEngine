#ifndef RY_ERRORLOG
#define RY_ERRORLOG

#include "Exceptions.h"

#include <fstream>
#include <string>
#include <map>
#include <memory>

class ErrorLog{;
    public:
        class Exception : public LEngineException{using LEngineException::LEngineException;};

        enum class SEVERITY{
            FATAL = 0,
            ERROR = 1,
            WARN = 2,
            INFO = 3,
            DEBUG = 4,
            TRACE = 5
        };

        static ErrorLog* Inst();
        static ErrorLog* pointertoself;

        static void OpenFile    (const std::string& fname);
        static void CloseFiles  ();

        static void WriteToFile (const std::string& text, SEVERITY severity, const std::string& fname = ErrorLog::GenericLogFile);
        static void WriteToFile (const std::string& text, const std::string& fname = ErrorLog::GenericLogFile);

        static const std::string GenericLogFile;
        static const std::string fileExtension;
        static const std::string logPath;

    protected:
        ErrorLog();
        static std::map<std::string, std::unique_ptr<std::ofstream> > errorFiles;

    private:
        static std::ofstream* GetFilePointer(const std::string& fname);
        static std::map<SEVERITY, std::string> SEVERITY_TAGS;
};

#endif
