#include "ResourceLoading.h"
#include "../Kernel.h"


std::unique_ptr<FileData> LoadGenericFile(const std::string& fileName){
    if(PHYSFS_exists(fileName.c_str())==0){
        std::stringstream ss;
        ss << "Couldn't find Generic file: " << fileName << "\n     Physfs Error: " << PHYSFS_getLastError();
        throw LEngineFileException(ss.str(), fileName);
    }

    PHYSFS_File *file= PHYSFS_openRead(fileName.c_str());
    PHYSFS_seek(file, 0);

    PHYSFS_sint64 fileSize=PHYSFS_fileLength(file);
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
        LOG_INFO(ss.str());
		throw LEngineFileException(ss.str(), fileName);
    }

    PHYSFS_close(file);

    return data;
}

FileData::FileData(unsigned int len) : length(len){
    if(length<=0){data=NULL;}
    else{
        data = new char [length];
    }
}
FileData::~FileData(){
    delete [] data;
}

char* FileData::GetData(){return data;}
