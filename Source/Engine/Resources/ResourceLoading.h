#ifndef L_RSC_LOADING
#define L_RSC_LOADING

#include "../Errorlog.h"
#include "../Defines.h"

#include "../physfs.h"
#include "RSC_Texture.h"
#include "RSC_Sprite.h"
#include "RSC_Sound.h"
#include "RSC_Script.h"
#include "RSC_Heightmap.h"
#include "RSC_Map.h"
#include "../Exceptions.h"

#include <string>
#include <memory>

class FileData{
    public:
        FileData(unsigned int len);
        ~FileData();
        char* GetData();

        const unsigned int length;

    private:
        char* data;

        //Cannot use copy assignment constructor
        FileData& operator= (const FileData& other) = delete;
};

class Kernel;//Forward Declare

std::unique_ptr<FileData>   LoadGenericFile (const std::string& fileName);
//LEvent*                     LoadEVENT   (const std::string& fname);


#endif
