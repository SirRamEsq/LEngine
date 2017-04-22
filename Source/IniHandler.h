#ifndef L_INIHANDLER
#define L_INIHANDLER

#include "Errorlog.h"

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cstring>
#include <sstream>

class IniHandler{
    typedef std::map<std::string, std::string> KeyValue;
    typedef KeyValue::iterator KVIt;
    public:
        IniHandler();
        ~IniHandler();

        template <typename T>
        T StringToNumber (const std::string& Text )//Text not by const reference so that the function can be used with a
        {                               //character array as argument
            std::stringstream ss(Text);
            T result;
            return ss >> result ? result : 0;
        }

        bool OpenReadFile (std::string& fname);
        bool OpenWriteFile(std::string& fname);

        template <typename T>
        T ReadValueNum (const std::string& key){
            KVIt i=readMap.find(key);
            if(i==readMap.end()){return 0;}
            return (StringToNumber<T>(i->second));
        }
        std::string ReadValueStr (const std::string& key){
            KVIt i=readMap.find(key);
            if(i==readMap.end()){return "";}
            return i->second;
        }
        int ReadASCIIValue(const std::string& key){
            std::string val=ReadValueStr(key);
            if(val==""){return -1;}
            return (int)val[0];
        }

        int         WriteInt   (std::string key, int value);
        bool        WriteBool  (std::string key, bool value);
        std::string WriteString(std::string key, std::string value);
        double      WriteDouble(std::string key, double value);

        bool ReadFileToMap();  //Read all key value pairs from the ini file into the read map
        bool WriteMapToFile(); //Write all key value pairs from the write map into the ini file

        void CloseReadFile();
        void CloseWriteFile();

        void ClearWriteMap();
        void ClearReadMap();

        KVIt readItBeg(){return readMap.begin();}
        KVIt readItEnd(){return readMap.end();}
        KVIt writeItBeg(){return writeMap.begin();}
        KVIt writeItEnd(){return writeMap.end();}

    private:
        KeyValue readMap;
        KeyValue writeMap;
        std::ifstream iniRead;
        std::ofstream iniWrite;
};

#endif
