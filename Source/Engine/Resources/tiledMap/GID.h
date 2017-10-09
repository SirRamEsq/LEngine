#ifndef LENGINE_TILED_GID
#define LENGINE_TILED_GID

#include <map>

//The GID Manager is used to re use resources that the tilemaps need
//So far, this only includes tile sets. backgrounds may later also be GID Enabled
//Tiled objects do not need to be GID enabled, they are all unique and don't need to be looked up with a Global ID

typedef unsigned long int GID;
class GIDManager;

//A GID is a global identifier that is used to look up data that is relevant to a certain resource
//A GID enabled class is a class that makes use of a range of GID values

//Once a GID enabled item is created, it's expected to not be deleted until the manager is
class GIDEnabled{
    public:
        GIDEnabled(GIDManager* man, GID first, GID last);
        GIDEnabled(GIDManager* man                     );

        void ReconcileToFirst();
        void ReconcileToLast ();

        GID GetFirstGID      () const ;
        GID GetLastGID       () const ;

        void SetFirstGID      (GID id);
        void SetLastGID       (GID id);

        void IncrementFirstGID(GID id);
        void IncrementLastGID (GID id);

    private:
        GID firstGID, lastGID;
        GIDManager* manager;
};

//This class keeps a record of what GID ranges belong to which objects
class GIDManager{
    public:
        class Range{
            public:
                Range(const GID& l, const GID& h);
                Range();

                Range& operator=(const Range& r);
                bool operator<(const Range& r) const;

                GID high, low;
        };

        Range       NewRange (const GID& first, const GID& last, GIDEnabled* item);
        GIDEnabled* GetItem  (const GID& index                                   ) const;

    private:
        //GIDEnabled items can be lookedup by Range
        std::map<Range,         GIDEnabled*> GIDItems;
};

#endif

