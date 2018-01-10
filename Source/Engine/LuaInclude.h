#ifndef L_ENGINE_LUA_INCLUDES
#define L_ENGINE_LUA_INCLUDES

#include <map>
#include <list>
#include <vector>
// Lua headers
extern "C" {
#include "lua5.2/lauxlib.h"
#include "lua5.2/lua.h"
#include "lua5.2/lualib.h"
}
#include <LuaBridge.h>

namespace Bindings {
/**
 * Converts Lua list-like table to std containers like vector, list.
 */
template <class T>
static T tableToList(const luabridge::LuaRef &table) {
  T list;
  if (table.isTable()) {
    for (size_t i = 1, n = (size_t)table.length();
         (i <= n && !table[i].isNil()); i++) {
      list.push_back(table[i]);  // implicit conversion from luabridge::Proxy
                                 // should be called here
    }
  }
  return list;
}

/**
 * Assigns values from containers like list/vector to Lua list-like Table.
 */
template <class T>
static void listToTable(luabridge::LuaRef &table, const T &list) {
  for (auto element : list) {
    table.append(element);
  }
}

/**
 * Converts Lua table to std::map containers.
 */
template <class T>
static T tableToMap(const luabridge::LuaRef &table) {
  T map;
  if (table.isTable()) {
    for (luabridge::Iterator iter(table); !iter.isNil(); ++iter) {
      map[iter.key()] = iter.value();  // implicit conversion from LuaRefs
                                       // should be called here
    }
  }
  return map;
}
/**
 * Assigns values from map containers to Lua Table.
 */
template <class T>
static void mapToTable(luabridge::LuaRef &table, const T &map) {
  for (auto element : map) {
    table[element.first] = element.second;
  }
}
}


namespace luabridge{
/**
 * Specializations for luabridge::Stack<T>.
 */

/**
 * Stack specialization for std::vector.
 * Creates new table every time vector is returned to Lua and new vector from
 * table other way around.
 */
template <>
template <typename T>
struct Stack<std::vector<T>> {
 public:
  static inline void push(lua_State *L, std::vector<T> vec) {
    auto table = LuaRef::newTable(L);
    Bindings::listToTable<std::vector<T>>(table, vec);
    table.push(L);
  }

  static inline std::vector<T> get(lua_State *L, int index) {
    return Bindings::tableToList<std::vector<T>>(LuaRef::fromStack(L, index));
  }
};

/**
 * Stack specialization for std::vector. Const reference version.
 */
template <>
template <typename T>
struct Stack<const std::vector<T> &> {
 public:
  static inline void push(lua_State *L, const std::vector<T> &vec) {
    auto table = LuaRef::newTable(L);
    Bindings::listToTable<std::vector<T>>(table, vec);
    table.push(L);
  }

  static inline std::vector<T> get(lua_State *L, int index) {
    return Bindings::tableToList<std::vector<T>>(LuaRef::fromStack(L, index));
  }
};

/**
 * Stack specialization for std::list. Converts list to Lua table and vice
 * versa.
 */
template <>
template <typename T>
struct Stack<std::list<T>> {
 public:
  static inline void push(lua_State *L, std::list<T> list) {
    auto table = LuaRef::newTable(L);
    Bindings::listToTable<std::list<T>>(table, list);
    table.push(L);
  }

  static inline std::list<T> get(lua_State *L, int index) {
    return Bindings::tableToList<std::list<T>>(LuaRef::fromStack(L, index));
  }
};

/**
 * Stack specialization for std::list. Const ref version.
 */
template <>
template <typename T>
struct Stack<const std::list<T> &> {
 public:
  static inline void push(lua_State *L, const std::list<T> &list) {
    auto table = LuaRef::newTable(L);
    Bindings::listToTable<std::list<T>>(table, list);
    table.push(L);
  }

  static inline std::list<T> get(lua_State *L, int index) {
    return Bindings::tableToList<std::list<T>>(LuaRef::fromStack(L, index));
  }
};

/**
 * Stack specialization for std::map. Converts map to Lua table and vice versa.
 */
template <>
template <typename TK, typename TV>
struct Stack<std::map<TK, TV>> {
 public:
  static inline void push(lua_State *L, std::map<TK, TV> map) {
    auto table = LuaRef::newTable(L);
    Bindings::mapToTable(table, map);
    table.push(L);
  }

  static inline std::map<TK, TV> get(lua_State *L, int index) {
    return Bindings::tableToMap<std::map<TK, TV>>(LuaRef::fromStack(L, index));
  }
};

/**
 * Stack specialization for std::map. Const ref version.
 */
template <>
template <typename TK, typename TV>
struct Stack<const std::map<TK, TV> &> {
 public:
  static inline void push(lua_State *L, const std::map<TK, TV> &map) {
    auto table = LuaRef::newTable(L);
    Bindings::mapToTable(table, map);
    table.push(L);
  }

  static inline std::map<TK, TV> get(lua_State *L, int index) {
    return Bindings::tableToMap<std::map<TK, TV>>(LuaRef::fromStack(L, index));
  }
};

}
#endif
