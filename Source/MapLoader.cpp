#include "MapLoader.h"
#include "Kernel.h"
#include "TileMap.h"

TileMap* LoadRMap(std::string fname){
    //PHYSFS_init(NULL);
    PHYSFS_addToSearchPath("Data/Resources", 0);
    if(PHYSFS_exists(fname.c_str())==0){
        ErrorLog::WriteToFile("Couldn't find map file: ", fname.c_str());
        ErrorLog::WriteToFile("Physfs Error: ", PHYSFS_getLastError());
        return NULL;
    }

    bool convert;
    if(GET_ENDIAN()==LITTLE_ENDIAN){convert=true;}
    else{convert=false;}

    char *strSize;
    char *file, *str;
    uint32_t width, height, dataInt, layers;
    int32_t version;
    int pointer=0;
    std::string bgname;
    std::string dependname;
    TileMap* tmap;

    PHYSFS_File *pfile= PHYSFS_openRead(fname.c_str());
    PHYSFS_seek(pfile, 0);
    uint32_t fileSize=PHYSFS_fileLength(pfile);
    if(fileSize<0){
        ErrorLog::WriteToFile("Couldn't Load map");
        return NULL;
    }
    file=new char[fileSize];
    int lengthRead=PHYSFS_read(pfile, file, 1, fileSize);
    if(lengthRead==-1){//read file into memory
        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////////////////

    strSize=(char*) &dataInt;

    strSize[0]=file[pointer]; pointer++; //Get version
    strSize[1]=file[pointer]; pointer++;
    strSize[2]=file[pointer]; pointer++;
    strSize[3]=file[pointer]; pointer++;

    if(convert==true){
        dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
    }
    version=dataInt;
    //////////////////////////////////////////////////////////////////////////////////////

    strSize=(char*) &dataInt;

    strSize[0]=file[pointer]; pointer++; //Get size of depend string
    strSize[1]=file[pointer]; pointer++;
    strSize[2]=file[pointer]; pointer++;
    strSize[3]=file[pointer]; pointer++;

    if(convert==true){
        dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
    }
    str= new char[dataInt];
    for(int i=0; i!=dataInt; i++){str[i]=file[pointer]; pointer++;} //get depend string
    dependname=std::string(str);
    delete [] str;

    ////////////////////////////////////////////////////////////////////////////////////////

    //Attempt to use the depend string
    dependname.append(".7z");
    if(!LoadResourceFile(dependname)){
        ErrorLog::WriteToFile("Error loading map file, couldn't find companion resource file");
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    strSize=(char*) &dataInt;

    strSize[0]=file[pointer]; pointer++; //Get width
    strSize[1]=file[pointer]; pointer++;
    strSize[2]=file[pointer]; pointer++;
    strSize[3]=file[pointer]; pointer++;

    if(convert==true){
        dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
    }
    width=dataInt;

    ////////////////////////////////////////////////////////////////////////////////////////

    strSize=(char*) &dataInt;

    strSize[0]=file[pointer]; pointer++; //Get height
    strSize[1]=file[pointer]; pointer++;
    strSize[2]=file[pointer]; pointer++;
    strSize[3]=file[pointer]; pointer++;

    if(convert==true){
        dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
    }
    height=dataInt;

    /////////////////////////////////////////////////////////////////////////////////////////

    strSize=(char*) &dataInt;

    strSize[0]=file[pointer]; pointer++; //Get depth (number of tile layers)
    strSize[1]=file[pointer]; pointer++;
    strSize[2]=file[pointer]; pointer++;
    strSize[3]=file[pointer]; pointer++;

    if(convert==true){
        dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
    }
    layers=dataInt;

    /////////////////////////////////////////////////////////////////////////////////////////

    tmap=new TileMap;
    tmap->Startup(width, height);

    std::map<std::string, LTDF*>::iterator ti=K_TDFMan->ItBegin();
    for(; ti!=K_TDFMan->ItEnd(); ti++){
        tmap->AddTileDef(ti->second->GetTDFName());
    }

    //Get Tiles
    std::string tile;
    int layernum=0;
    for(int depth=0; depth<layers; depth++){
        //READ LAYERNUM
        strSize=(char*) &dataInt;

        strSize[0]=file[pointer]; pointer++;
        strSize[1]=file[pointer]; pointer++;
        strSize[2]=file[pointer]; pointer++;
        strSize[3]=file[pointer]; pointer++;
        if(convert==true){
            dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
        }
        layernum=dataInt;

        tmap->InitTileLayer(layernum);

        for(uint32_t y=0; y<height; y++){
            for(uint32_t x=0; x<width; x++){

                strSize=(char*) &dataInt;

                strSize[0]=file[pointer]; pointer++; //Get size of tile string
                strSize[1]=file[pointer]; pointer++;
                strSize[2]=file[pointer]; pointer++;
                strSize[3]=file[pointer]; pointer++;

                if(convert==true){
                    dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
                }
                str= new char[dataInt];
                for(int i=0; i!=dataInt; i++){str[i]=file[pointer]; pointer++;} //get tile string
                tile=std::string(str);
                delete [] str;

                ////////////////////////////////////////////////////////

                tmap->SetTile(tile,x,y,layernum);
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    uint32_t objcount;

    strSize=(char*) &dataInt;

    strSize[0]=file[pointer]; pointer++; //Get number of objects
    strSize[1]=file[pointer]; pointer++;
    strSize[2]=file[pointer]; pointer++;
    strSize[3]=file[pointer]; pointer++;

    if(convert==true){
        dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
    }
    objcount=dataInt;

    std::string objname;
    int objx, objy, objdepth;
    LObj* obj;
    LSprite* spr;
    LScript* scr;
    EID eid;
    for(uint32_t i=0; i!=objcount; i++){
        strSize=(char*) &dataInt;

        strSize[0]=file[pointer]; pointer++; //Get size of name string
        strSize[1]=file[pointer]; pointer++;
        strSize[2]=file[pointer]; pointer++;
        strSize[3]=file[pointer]; pointer++;

        if(convert==true){
            dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
        }
        str= new char[dataInt];
        for(int i=0; i!=dataInt; i++){str[i]=file[pointer]; pointer++;} //get name string
        objname=std::string(str);
        delete [] str;

        ////////////////////////////////////////////////////////

        strSize=(char*) &dataInt;

        strSize[0]=file[pointer]; pointer++; //Get objx
        strSize[1]=file[pointer]; pointer++;
        strSize[2]=file[pointer]; pointer++;
        strSize[3]=file[pointer]; pointer++;

        if(convert==true){
            dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
        }
        objx=dataInt;

        /////////////////////////////////////////////////////////

        strSize=(char*) &dataInt;

        strSize[0]=file[pointer]; pointer++; //Get objx
        strSize[1]=file[pointer]; pointer++;
        strSize[2]=file[pointer]; pointer++;
        strSize[3]=file[pointer]; pointer++;

        if(convert==true){
            dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
        }
        objy=dataInt;

        /////////////////////////////////////////////////////////

        strSize=(char*) &dataInt;

        strSize[0]=file[pointer]; pointer++; //Get objdepth
        strSize[1]=file[pointer]; pointer++;
        strSize[2]=file[pointer]; pointer++;
        strSize[3]=file[pointer]; pointer++;

        if(convert==true){
            dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
        }
        objdepth=dataInt;

        /////////////////////////////////////////////////////////

        obj=K_ObjMan->GetItem(objname);
        if(obj!=NULL){
            eid=K_EntMan->NewEntity();
            K_ComPosMan->AddComponent(eid);
            ((ComponentPosition*)K_ComPosMan->GetComponent(eid))->SetPosition( Coord2df(objx,objy) );

            /*spr=K_SpriteMan->GetItem(obj->spriteName);
            if(spr!=NULL){
                K_ComSpriteMan->AddComponent(eid);
                ((ComponentSprite*)K_ComSpriteMan->GetComponent(eid))->SetCurrentSprite(spr);
                ((ComponentSprite*)K_ComSpriteMan->GetComponent(eid))->SetDepth(objdepth);
                spr=((ComponentSprite*)K_ComSpriteMan->GetComponent(eid))->GetSprite();
                spr->SetCurrentAnimation(obj->animationName);
            }*/

            scr=K_ScriptMan->GetItem(obj->scriptName);
            if(scr!=NULL){
                K_ComScriptMan->
                    AddComponent(eid);
                K_ComScriptMan->DoFile(eid, scr);
            }
            else{
                ErrorLog::WriteToFile("Couldn't find script named ", obj->scriptName);
            }
        }
        else{
            ErrorLog::WriteToFile("Couldn't load object into map. LObj resource wasn't found. Object name is: ", objname);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    uint32_t bgcount;

    strSize=(char*) &dataInt;

    strSize[0]=file[pointer]; pointer++; //Get number of backgrounds
    strSize[1]=file[pointer]; pointer++;
    strSize[2]=file[pointer]; pointer++;
    strSize[3]=file[pointer]; pointer++;

    if(convert==true){
        dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
    }
    bgcount=dataInt;

    std::string bgName;
    int bgdepth;
    for(uint32_t i=0; i!=bgcount; i++){
        strSize=(char*) &dataInt;

        strSize[0]=file[pointer]; pointer++; //Get size of bgname string
        strSize[1]=file[pointer]; pointer++;
        strSize[2]=file[pointer]; pointer++;
        strSize[3]=file[pointer]; pointer++;

        if(convert==true){
            dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
        }
        str= new char[dataInt];
        for(int i=0; i!=dataInt; i++){str[i]=file[pointer]; pointer++;} //get bgname string
        bgName=std::string(str);
        delete [] str;

        /////////////////////////////////////////////////////////

        strSize=(char*) &dataInt;

        strSize[0]=file[pointer]; pointer++; //Get bgdepth
        strSize[1]=file[pointer]; pointer++;
        strSize[2]=file[pointer]; pointer++;
        strSize[3]=file[pointer]; pointer++;

        if(convert==true){
            dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
        }
        bgdepth=dataInt;

        //////////////////////////////////////////////////////////

        tmap->AddBackground(new MapBackground(bgName, bgdepth));
    }
    ////////////////////////////////////////////////////////////////////////////////////////

    uint32_t evecount;

    strSize=(char*) &dataInt;

    strSize[0]=file[pointer]; pointer++; //Get number of events
    strSize[1]=file[pointer]; pointer++;
    strSize[2]=file[pointer]; pointer++;
    strSize[3]=file[pointer]; pointer++;

    if(convert==true){
        dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
    }
    evecount=dataInt;

    std::string eventName;
    int eventdepth,x,y,w,h;
    for(uint32_t i=0; i!=evecount; i++){
        strSize=(char*) &dataInt;

        strSize[0]=file[pointer]; pointer++; //Get size of eventname string
        strSize[1]=file[pointer]; pointer++;
        strSize[2]=file[pointer]; pointer++;
        strSize[3]=file[pointer]; pointer++;

        if(convert==true){
            dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
        }
        str= new char[dataInt];
        for(int i=0; i!=dataInt; i++){str[i]=file[pointer]; pointer++;} //get eventname string
        eventName=std::string(str);
        delete [] str;

        /////////////////////////////////////////////////////////

        strSize=(char*) &dataInt;

        strSize[0]=file[pointer]; pointer++; //Get x
        strSize[1]=file[pointer]; pointer++;
        strSize[2]=file[pointer]; pointer++;
        strSize[3]=file[pointer]; pointer++;

        if(convert==true){
            dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
        }
        x=dataInt;

        /////////////////////////////////////////////////////////

        strSize=(char*) &dataInt;

        strSize[0]=file[pointer]; pointer++; //Get y
        strSize[1]=file[pointer]; pointer++;
        strSize[2]=file[pointer]; pointer++;
        strSize[3]=file[pointer]; pointer++;

        if(convert==true){
            dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
        }
        y=dataInt;

        /////////////////////////////////////////////////////////

        strSize=(char*) &dataInt;

        strSize[0]=file[pointer]; pointer++; //Get w
        strSize[1]=file[pointer]; pointer++;
        strSize[2]=file[pointer]; pointer++;
        strSize[3]=file[pointer]; pointer++;

        if(convert==true){
            dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
        }
        w=dataInt;

        /////////////////////////////////////////////////////////

        strSize=(char*) &dataInt;

        strSize[0]=file[pointer]; pointer++; //Get h
        strSize[1]=file[pointer]; pointer++;
        strSize[2]=file[pointer]; pointer++;
        strSize[3]=file[pointer]; pointer++;

        if(convert==true){
            dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
        }
        h=dataInt;

        /////////////////////////////////////////////////////////

        strSize=(char*) &dataInt;

        strSize[0]=file[pointer]; pointer++; //Get eventdepth
        strSize[1]=file[pointer]; pointer++;
        strSize[2]=file[pointer]; pointer++;
        strSize[3]=file[pointer]; pointer++;

        if(convert==true){
            dataInt=INT32_REVERSE_BYTES((int32_t)dataInt);
        }
        eventdepth=dataInt;

        //////////////////////////////////////////////////////////

        tmap->AddEvent(new MapEvent(eventName, x,y,w,h, eventdepth));
    }
    ////////////////////////////////////////////////////////////////////////////////////////

    delete [] file;

    PHYSFS_close(pfile);
    PHYSFS_deinit();
    return tmap;
}

TileMap* LoadLuaMap(LMap* lmap){

}
