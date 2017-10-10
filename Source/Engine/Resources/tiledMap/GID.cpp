#include "GID.h"
#include "../../Kernel.h"

GIDEnabled::GIDEnabled(GIDManager* man, GID first, GID last)
    : manager(man), firstGID(first), lastGID(last){
    if(firstGID>lastGID){LOG_INFO("[C++] GIDEnabled Constructor: lowest gid passed is greater than highest GID passed");}

    manager->NewRange(first, last, this);
}

GIDEnabled::GIDEnabled(GIDManager* man)
    : manager(man){
    firstGID=0;
    lastGID=0;
}

void GIDEnabled::ReconcileToFirst(){
	if(firstGID > lastGID){lastGID = firstGID;}
}

void GIDEnabled::ReconcileToLast (){
	if(lastGID < firstGID){firstGID = lastGID;}
}

GID GIDEnabled::GetFirstGID() const{
	return firstGID;
}

GID GIDEnabled::GetLastGID() const {
	return lastGID;
}

void GIDEnabled::SetFirstGID(GID id){
	firstGID = id;
	ReconcileToFirst();
}

void GIDEnabled::SetLastGID(GID id){
	lastGID = id;
	ReconcileToLast();
}

void GIDEnabled::IncrementFirstGID(GID id){
	firstGID += id;
	ReconcileToFirst();
}

void GIDEnabled::IncrementLastGID (GID id){
	lastGID += id;
	ReconcileToLast();
}

//////////////
//GIDManager//
//////////////

GIDManager::Range GIDManager::NewRange(const GID& first, const GID& last, GIDEnabled* item){
    Range r(first, last);
    GIDItems[r]=item;
    return r;
}

GIDEnabled* GIDManager::GetItem (const GID& index) const {
    for(auto i=GIDItems.begin(); i!=GIDItems.end(); i++){
        if((i->first.low <= index) and (i->first.high >= index)){
            return i->second;
        }
    }
    return NULL;
}
GIDManager::Range::Range(const GID& l, const GID& h) {
	high=h;
	low=l;
}

GIDManager::Range::Range(){
	high=0;
	low=100;
}

GIDManager::Range& GIDManager::Range::operator=(const GIDManager::Range& r) {
	if (this!=&r){  // Same object?
		low=r.low;
		high=r.high;
	}
	return *this;
}

bool GIDManager::Range::operator<(const GIDManager::Range& r) const{
	if (low < r.low){
		assert(high < r.low); // sanity check
		return true;
	}
	return false;
}
