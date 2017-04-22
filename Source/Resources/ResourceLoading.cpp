#include "ResourceLoading.h"
#include "../Kernel.h"

LSprite* LoadSPR(const std::string& fname){
    LSprite* sprite = NULL;
    try{
        std::string fullPath = "Resources/Sprites/"+fname;
        auto data=LoadGenericFile(fullPath);
        if(data.get()->GetData()==NULL){
            return NULL;
        }
        sprite = new LSprite(fname);
        if(sprite->LoadFromXML(data.get()->GetData(), data.get()->length)==false){
            ErrorLog::WriteToFile("Couldn't parse XML Sprite Data for sprite " + fname, ErrorLog::GenericLogFile);
            delete sprite;
            return NULL;
        }
    }
    catch(LEngineFileException e){
        ErrorLog::WriteToFile(e.what(), ErrorLog::GenericLogFile);
    }

    return sprite;
}

LMusic* LoadMUS(const std::string& fname){
    LMusic* music = NULL;
    try{
        std::string fullPath = "Resources/Music/"+fname;
        auto data=LoadGenericFile(fullPath);
        if(data.get()->GetData()==NULL){
            return NULL;
        }
        music = new LMusic(fname, data.get()->GetData(), data.get()->length);
    }
    catch(LEngineFileException e){
        ErrorLog::WriteToFile(e.what(), ErrorLog::GenericLogFile);
    }

    return music;
}

LSound* LoadSND(const std::string& fname){
    LSound* sound = NULL;
    try{
        std::string fullPath = "Resources/Sounds/"+fname;
        auto data=LoadGenericFile(fullPath);
        if(data.get()->GetData()==NULL){
            return NULL;
        }
        sound = new LSound(fname, data.get()->GetData(), data.get()->length);
    }
    catch(LEngineFileException e){
        ErrorLog::WriteToFile(e.what(), ErrorLog::GenericLogFile);
    }

    return sound;
}

LTexture* LoadTEX(const std::string& fname){
    LTexture* texture = NULL;
    try{
        std::string fullPath = "Resources/Images/"+fname;
        auto data=LoadGenericFile(fullPath);
        if(data.get()->GetData()==NULL){
            return NULL;
        }
        texture = new LTexture((const unsigned char*)data.get()->GetData(), data.get()->length, fname);
        texture->SetColorKey(MASK_R, MASK_G, MASK_B, true);
    }
    catch(LEngineFileException e){
        ErrorLog::WriteToFile(e.what(), ErrorLog::GenericLogFile);
    }

    return texture;

}

LScript* LoadSCRIPT(const std::string& fname){
    LScript* script = NULL;
    try{
        std::string fullPath = "Resources/Scripts/"+fname;
        auto data=LoadGenericFile(fullPath);
        if(data.get()->GetData()==NULL){
            return NULL;
        }
        script = new LScript(fname, data.get()->GetData(), data.get()->length);
    }
    catch(LEngineFileException e){
        ErrorLog::WriteToFile(e.what(), ErrorLog::GenericLogFile);
    }

    return script;
}


LEvent* LoadEVENT(const std::string& fname){
    std::string strfname;
    //Look in the "Resources" folder in the zip file
    strfname="Resources/"+fname;
    if(PHYSFS_exists(strfname.c_str())==0){
        std::stringstream ss;
        ss << "Couldn't find event file: " << strfname.c_str() << "\n"
        << "Physfs Error: " << PHYSFS_getLastError();
        ErrorLog::WriteToFile(ss.str(), ErrorLog::GenericLogFile);
        return NULL;
    }

    std::string eventScriptName;
    int32_t flags, dataInt;
    char *strSize, *str, *file;
    int pointer=0;
    bool convert;
    if(GET_ENDIAN()==LENGINE_DEF_LITTLE_ENDIAN){convert=true;}
    else{convert=false;}

    PHYSFS_File *pfile= PHYSFS_openRead(strfname.c_str());
    PHYSFS_seek(pfile, 0);
    uint32_t fileSize=PHYSFS_fileLength(pfile);
    if(fileSize<0){
        ErrorLog::WriteToFile("Couldn't Load event", ErrorLog::GenericLogFile);
        return NULL;
    }
    file=new char[fileSize];
    int lengthRead=PHYSFS_read(pfile, file, fileSize, 1);
    if(lengthRead==-1){//read file into memory
        return NULL;
    }

    //fname.resize(fname.size () - 6);//Get rid of the .rlevt extension

    ////////////////////////////////////////////////////////

    strSize=(char*) &dataInt;

    strSize[0]=file[pointer]; pointer++; //Get size of scriptname string
    strSize[1]=file[pointer]; pointer++;
    strSize[2]=file[pointer]; pointer++;
    strSize[3]=file[pointer]; pointer++;

    if(convert==true){
        dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
    }
    str= new char[dataInt];
    for(int i=0; i!=dataInt; i++){str[i]=file[pointer]; pointer++;} //get scriptname string
    eventScriptName=std::string(str);
    delete [] str;

    /////////////////////////////////////////////////////////

    strSize=(char*) &dataInt;

    strSize[0]=file[pointer]; pointer++; //Get eventflags
    strSize[1]=file[pointer]; pointer++;
    strSize[2]=file[pointer]; pointer++;
    strSize[3]=file[pointer]; pointer++;

    if(convert==true){
        dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
    }
    flags=dataInt;

    delete [] file;
    PHYSFS_close(pfile);

    return new LEvent(eventScriptName, flags);
}

LMap* LoadMAP(const std::string& fname){
    LMap* lmap= NULL;
    try{
        std::string fullPath= "Resources/Maps/"+fname;
        auto data=LoadGenericFile(fullPath);
        if(data.get()->GetData()==NULL){
            return NULL;
        }
        lmap= new LMap(fname, data.get()->GetData(), data.get()->length);
    }
    catch(LEngineFileException e){
        ErrorLog::WriteToFile(e.what(), ErrorLog::GenericLogFile);
    }

    return lmap;
}

std::unique_ptr<FileData> LoadGenericFile(const std::string& fileName){
    if(PHYSFS_exists(fileName.c_str())==0){
        std::stringstream ss;
        ss << "Couldn't find Generic file: " << fileName << "\n     Physfs Error: " << PHYSFS_getLastError();
        throw LEngineFileException(ss.str(), fileName);
    }

    PHYSFS_File *file= PHYSFS_openRead(fileName.c_str());
    PHYSFS_seek(file, 0);

    uint64_t fileSize=PHYSFS_fileLength(file);
    if(fileSize<0){
        PHYSFS_close(file);
        throw LEngineFileException("Couldn't Load Generic Item", fileName);
    }

    std::unique_ptr<FileData> data(new FileData(fileSize));
    int lengthRead=PHYSFS_read(file, data.get()->GetData(), fileSize, 1);
    if(lengthRead==-1){//if read failed
        PHYSFS_close(file);
        throw LEngineFileException("Reading Generic Item data failed", fileName);
    }

    const char * physfsError = PHYSFS_getLastError();
    if(physfsError!=NULL){
        std::stringstream ss;
        ss << "Physfs Error in Generic File '" << fileName << "' Error: " << physfsError;
        ErrorLog::WriteToFile(ss.str(), ErrorLog::GenericLogFile);
    }

    PHYSFS_close(file);

    return data;
}

FileData::FileData(unsigned int len) : length(len){
    if(length==0){data=NULL;}
    else{
        data = new char [length];
    }
}
FileData::~FileData(){
    delete [] data;
}

char* FileData::GetData(){return data;}
