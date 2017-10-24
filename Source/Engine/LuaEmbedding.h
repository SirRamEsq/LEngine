#ifndef L_LUA_EMBEDDING
#define L_LUA_EMBEDDING

#include "Luna.hpp"

template <class H, class T>
struct TypeList {
  typedef H head;
  typedef T tail;
};

struct nulltype {};

/*=================================================================*/
template <typename T, typename U>
struct is_same {
  // Default case: T and U are not the same type
  static const bool value = false;
};
template <typename T>
struct is_same<T, T> {
  // Specialization: both template arguments are of the same type
  static const bool value = true;
};
/*=================================================================*/
template <typename TList, typename T>
struct contains {
  static const bool value =
      is_same<typename TList::head, T>::value       // Base case
      || contains<typename TList::tail, T>::value;  // Recursion
};
template <typename T>
struct contains<nulltype, T> {
  // Termination condition
  static const bool value = false;
};
/*=================================================================*/
template <bool Cond, typename T = void>
struct enable_if {
  // Default case: Cond assumed to be false, no typedef
};
template <typename T>
struct enable_if<true, T> {
  // Specialization: Cond is true, so typedef
  typedef T type;
};
/*=================================================================*/

template <class TList>
struct IndexOf {
 private:
  enum { temp = IndexOf<typename TList::tail>::value };

 public:
  enum { value = 1 + temp };
};

template <>
struct IndexOf<nulltype> {
  enum { value = -1 };
};

template <typename TList>
struct length {
  int GetLength(int i = 0) {
    length<typename TList::tail> l;
    return l->GetLength(i + 1);
  }
};
template <>
struct length<nulltype> {
  // Termination condition
  // static const int value = i;
  int GetLength(int i) { return i; }
};

// This class is used to transfer lua data instead of raw pointers
/*Luadata* is basically a pointer to a pointer, thus the Garbage Collector won't
eat the real data
template <class T>
struct LuaData{
    LuaData(){luaOwns=false;}
    T* pToCPP;
    bool luaOwns;
};

void GC(lua_State* l){
    LuaData* ld;
    ld = reinterpret_cast<LuaData*>(lua_touserdata(l, 1));
    if(ld->luaOwns==true){
        delete ld->pToCPP
        ld->pToCPP = NULL;
    }
}

template <class T>
T LGet_Value(lua_State* ls, int stackpos);

template <class T>
T LPush_Value(lua_State* ls, int stackpos);*/

// Super crazy Macro Hack time
#define REGISTER_CLASS(ls, cName)                  \
  luaL_newtable(ls);                               \
  /*Get the id of the newly created table*/        \
  int methods = lua_gettop(l);                     \
  /*Hook up table, name it after the class*/       \
  luaL_newmetatable(l, #cName);                    \
  /*Get the id of the newly created metatable*/    \
  int metatable = lua_gettop(l);                   \
                                                   \
  /*Push the name of the class onto the stack*/    \
  lua_pushstring(l, #cName);                       \
  /*Push the new table onto the stack*/            \
  lua_pushvalue(l, methods);                       \
  /*Set the table name to be the class name*/      \
  /* in the Global Environment*/                   \
  lua_settable(l, LUA_GLOBALSINDEX);               \
  /*in lua, #cName now equals this table*/         \
                                                   \
  /*Hook up metatable to the table*/               \
  lua_pushliteral(l, "__metatable");               \
  lua_pushvalue(l, methods);                       \
  lua_settable(l, metatable);                      \
  /*lua table #cName now looks for*/               \
  /*methods in this metatable*/                    \
                                                   \
  /*Register a Garbage Collector(Destructor) for*/ \
  /*the metatable*/                                \
  lua_pushliteral(l, "__gc");                      \
  lua_pushcfunction(l, cName::~cName);             \
  lua_settable(l, metatable);

#define REGISTER_CLASS_CONSTRUCTOR

#define REGISTER_CLASS_FUNCTION(ls, cName, function, typelist)

// YEEEEEHHHHHHHAAAAAAAAAAAAAAAAAW!!!

#endif
