#ifndef L_RSC_LOADING
#define L_RSC_LOADING

#include "../Errorlog.h"
#include "../Defines.h"

#include "../physfs.h"
#include "LTexture.h"
#include "LSprite.h"
#include "LSound.h"
#include "LScript.h"
#include "LObj.h"
#include "LTDF.h"
#include "LHeightmap.h"
#include "LBackground.h"
#include "LEvent.h"
#include "LMap.h"
#include "../Exceptions.h"

#include <string>

class FileData{
    public:
        FileData(unsigned int len);
        ~FileData();
        char* GetData();

        const unsigned int length;

    private:
        char* data;

        //Cannot use copy assignment constructor
        FileData& operator= (const FileData& other){}
};

class Kernel;//Forward Declare

std::unique_ptr<FileData> LoadGenericFile (const std::string& fileName);

LSprite*        LoadSPR     (const std::string& fname);
LMusic*         LoadMUS     (const std::string& fname);
LSound*         LoadSND     (const std::string& fname);
LTexture*       LoadTEX     (const std::string& fname);
LScript*        LoadSCRIPT  (const std::string& fname);
LEvent*         LoadEVENT   (const std::string& fname);
LMap*           LoadMAP     (const std::string& fname);


#endif
