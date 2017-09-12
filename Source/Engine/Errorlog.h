#ifndef RY_ERRORLOG
#define RY_ERRORLOG

#include "Exceptions.h"
#include "physfs.h"

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

		///If called, all entries will henceforth be written to this file
		void WriteToFile(const std::string& fileName);


		///Default value for Entry::type
        static const std::string typeDefault;
		///fileExtension used when saving to file
        static const std::string fileExtension;
		///file path used when saving to file
        static const std::string logPath;

		///Stores string reprentation of different severity levels
        static std::map<SEVERITY, std::string> SEVERITY_STR;
		///Will write the entries to a file
		static void WriteEntriesToFile(const std::vector<const Entry*> _entries, const std::string& fileName);
    protected:
		///All entries written to this log
		mutable std::vector<Entry> entries;

    private:
		///Filter used when calling GetEntries
		fp_EntryFilter entryFilter;
		int filterFlags;
        std::ofstream* GetFilePointer(const std::string& fname);

		PHYSFS_File* fileHandle; 

		///Closes currently opened file
		void CloseFileHandle();

		///Returns the date formated specially for fileNames
		static std::string GetDateString();
};

#endif
