#ifndef L_GENERIC_CONTAINER
#define L_GENERIC_CONTAINER

#include <map>
#include <string>
#include <memory>

#include "Errorlog.h"

template <class T>
class GenericContainer{
    typedef std::map<std::string, std::unique_ptr<const T> > tMap;
    typedef typename tMap::iterator mapIt;

    typedef std::unique_ptr<T> (*LoadFunction)(const std::string&); //function pointer called "LoadFunction"

    public:
        GenericContainer();
        ~GenericContainer();

        void SetLoadFunction(LoadFunction func);

        const T*           GetItem      (const std::string& name);
        std::unique_ptr<T> GetItemCopy  (const std::string& name);

        //Load item from pointer and store under 'name'
        bool LoadItem   (const std::string& name, std::unique_ptr<const T>& item);
        //Load item from filesystem with 'fname' as path and store it under 'name'
        bool LoadItem   (const std::string& name, const std::string& fname);
        //Get Item if it exists, try loading if it doesn't
        const T* GetLoadItem   (const std::string& name, const std::string& fname);
        void DeleteItem (const std::string& name);

        mapIt ItBegin(){return items.begin();}
        mapIt ItEnd()  {return items.end();  }
        int GetSize()  {return items.size(); }

    private:
        tMap items;
        LoadFunction function;
};

template <class T>
void GenericContainer<T>::SetLoadFunction(LoadFunction func){
    function=func;
}

template <class T>
GenericContainer<T>::GenericContainer(){
    function=NULL;
}

template <class T>
GenericContainer<T>::~GenericContainer(){
    items.clear();
}

template <class T>
const T* GenericContainer<T>::GetItem(const std::string& name){
    mapIt i=items.find(name);
    if(i!=items.end()){
       return (*i).second.get();
    }
    return NULL;
}

template <class T>
std::unique_ptr<T> GenericContainer<T>::GetItemCopy(const std::string& name){
    mapIt i=items.find(name);
    if(i!=items.end()){
       return std::unique_ptr<T> ((*i).second.get());
    }
    return NULL;
}

template <class T>
bool GenericContainer<T>::LoadItem(const std::string& name, std::unique_ptr<const T>& item){
    mapIt i=items.find(name);
    if(i!=items.end()){
       ErrorLog::WriteToFile("Generic Container already contains an item called", name);
       return false;
    }

    items[name]=std::unique_ptr<const T>(item.release());
    return true;
}

template <class T>
bool GenericContainer<T>::LoadItem(const std::string& name, const std::string& fname){
    mapIt i=items.find(name);
    if(i!=items.end()){
       ErrorLog::WriteToFile("Generic Container already contains an item called ", name);
       return false;
    }

    if(function==NULL){
        ErrorLog::WriteToFile("Generic Container doesn't have a function specified to load the resource ", name);
        return false;
    }

    std::unique_ptr<const T> newItem (function(fname));
    if(newItem.get()==NULL){
        ErrorLog::WriteToFile("Couldn't find resource named: ", name);
        return false;
    }

    items[name]=std::move(newItem);
    return true;
}

template <class T>
void GenericContainer<T>::DeleteItem(const std::string& name){
    mapIt i=items.find(name);
    if(i==items.end()){return;}

    else{
        items.erase(i);
    }
}

/** \breif Will try to get then load specified resource
 *
 * \param name of resource
 * \param fpath of resource
 * \return
 *
 */
template <class T>
const T* GenericContainer<T>::GetLoadItem   (const std::string& name, const std::string& fname){
    const T* item=GetItem(name);
    if(item==NULL){
        if(LoadItem(name,fname)==false){
            ErrorLog::WriteToFile("Could not get or load item named " + name + " At path " + fname, ErrorLog::GenericLogFile);
            return NULL;
        }
        item=GetItem(name);
    }
    return item;
}
#endif
