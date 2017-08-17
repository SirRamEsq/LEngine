#ifndef L_INIHANDLER
#define L_INIHANDLER

#include "Errorlog.h"
#include "Defines.h"

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cstring>
#include <sstream>

/**
 * Class is used to read and write key/value pairs from/to a .txt file
 *
 * Can have two files open, one for reading and one for writing
 * 
 * \note
 * INI File must take the form
 * key = value;
 * key=value;
 * key =value;
 * key= value;
 * end each line with a ';'
 */
class IniHandler{
    typedef std::map<std::string, std::string> KeyValue;
    public:
        IniHandler();
        ~IniHandler();

		///Opens a file to read K/v pairs from
        bool OpenReadFile (const std::string& fname);
		///Opens a file to write k/v pairs to
        bool OpenWriteFile(const std::string& fname);

		///Converts a value to a string, then adds it to the writeMap of k/v pairs
        template <typename T>
        T WriteValue (const std::string& key, T value){
			return WriteString(key, NumberToString<T>(value));
        }
		///Adds a k/v pair to the write map
        bool WriteString(const std::string& key, const std::string& value);

  		///Read all key value pairs from the ini file into the read map
        bool ReadFileToMap();
 		///Write all key value pairs from the write map into the ini file
        bool WriteMapToFile();

		///Closes read file if one is open
        void CloseReadFile();
		///Closes write file if one is open
        void CloseWriteFile();

		///Clears key/value pairs to be written to the open write file
        void ClearWriteMap();
		///Clears key/value pairs read from an opened read file
        void ClearReadMap();

		KeyValue::iterator readItBeg(){return readMap.begin();}
		KeyValue::iterator readItEnd(){return readMap.end();}
		KeyValue::iterator writeItBeg(){return writeMap.begin();}
		KeyValue::iterator writeItEnd(){return writeMap.end();}

		///Copies the map loaded into the read map into the write map
		void CopyReadMapToWriteMap();

    private:
        KeyValue readMap;
        KeyValue writeMap;
        std::ifstream iniRead;
        std::ofstream iniWrite;
};

#endif
