#ifndef RY_ERRORLOG
#define RY_ERRORLOG

#include "Exceptions.h"

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <inttypes.h>

class Log{;
    public:
        class Exception : public LEngineException{using LEngineException::LEngineException;};
        enum SEVERITY{
            FATAL = 1,
            ERROR = 2,
            WARN = 4,
            INFO = 8,
            DEBUG = 16,
            TRACE = 32 
        };

		struct Entry{
			Entry(const std::string& text, Log::SEVERITY s, const std::string& ty, uint32_t time);

			Log::SEVERITY severity;
			std::string type;
			std::string message;
			uint32_t timeStamp;

			std::string ToString() const;
		};

		///Signature of function used to determine if an entry is filtered or not
		typedef bool (*fp_EntryFilter)(const Entry& entry, int flags, int itemCount);


		Log();
        ~Log();

		///Set filter to use when calling GetEntries
		void SetEntryFilter(fp_EntryFilter filter);
		///Set filter flags to use for different severity levels
		void SetEntryFilterFlags(int flags);
		///Returns all entries. Will use filter set by SetEntryFilter if one was passed
		std::vector<const Entry*> GetEntries() const;

		/**
		 * Will write a new entry to entries
		 * \param text Text to write
		 * \param severity Severity of message
		 * \param type Optional metadata about the Entry's type
		 */
        void Write (const std::string& text, SEVERITY severity = SEVERITY::INFO, const std::string& type = Log::typeDefault) const;

		///Will write the entries to a file
		void WriteToFile(const std::vector<const Entry*> _entries, const std::string& fileName) const;

		///Default value for Entry::type
        static const std::string typeDefault;
		///fileExtension used when saving to file
        static const std::string fileExtension;
		///file path used when saving to file
        static const std::string logPath;

		///Stores string reprentation of different severity levels
        static std::map<SEVERITY, std::string> SEVERITY_STR;
    protected:
		///All entries written to this log
		mutable std::vector<Entry> entries;

    private:
		///Filter used when calling GetEntries
		fp_EntryFilter entryFilter;
		int filterFlags;
        std::ofstream* GetFilePointer(const std::string& fname);

};
/*
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

        static void WriteToFile (const std::string& text, SEVERITY severity, const std::string& fname = Log::typeDefault);
        static void WriteToFile (const std::string& text, const std::string& fname = Log::typeDefault);

        static const std::string GenericLogFile;
        static const std::string fileExtension;
        static const std::string logPath;

        static bool noThrow;

    protected:
        ErrorLog();
        static std::map<std::string, std::unique_ptr<std::ofstream> > errorFiles;

    private:
        static std::ofstream* GetFilePointer(const std::string& fname);
        static std::map<SEVERITY, std::string> SEVERITY_TAGS;
};
*/
#endif
