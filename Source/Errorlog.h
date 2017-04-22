#ifndef RY_ERRORLOG
#define RY_ERRORLOG

#include <fstream>
#include <string>
#include <map>

class ErrorLog{
    private:
        static std::ofstream* GetFilePointer(const std::string& fname);

    protected:
        ErrorLog();
        static std::map<std::string, std::ofstream*> errorFiles;

    public:
        static ErrorLog* Inst();
        static ErrorLog* pointertoself;

        static void OpenFile    (const std::string& fname);
        static void CloseFiles  ();

        static void WriteToFile (const std::string& text, const std::string& fname = ErrorLog::GenericLogFile);

        static const std::string GenericLogFile;
        static const std::string fileExtension;
        static const std::string logPath;
};

#endif
