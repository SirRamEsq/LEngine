/**
 * Sample binding to the Foo class with the Foo::call method.
 *
 * class Foo {
 * public:
 *   // Declares the types required by Luna (must be public)
 *   LUNA_TYPES(Foo);
 *
 *   Foo(lua_State* state) {
 *     ...
 *   }
 *
 *   int call(lua_State* state) {
 *     ...
 *     return 0;
 *   }
 * };
 *
 * // Binds the C++ class Foo using the Lua name Foo in Lua via the LuaFoo
 * // binding and exposing the call method.
 * LUNA_CLASS(LuaFoo, Foo, Foo) = {
 *     LUNA_METHOD(LuaFoo, call),
 *     {0}
 * };
 */

#ifndef LUNA_HPP
#define LUNA_HPP

#include <cstring>
#include <typeinfo>

#include <lua.h>
//used to be #include <lua.hpp>

#define LUNA_TYPES(T) \
    static const std::type_info* s_lunaTypeInfoPtr; \
    static const char s_lunaClassName[]; \
    static Luna<T>::MethodMap s_lunaMethods[]

#define LUNA_CLASS(T, MAPS_T, CLASS_NAME) \
    const std::type_info* T::s_lunaTypeInfoPtr(&typeid(MAPS_T)); \
    const char T::s_lunaClassName[] = #CLASS_NAME; \
    Luna<T>::MethodMap T::s_lunaMethods[]

#define LUNA_METHOD(T, NAME) {#NAME, &T::NAME}

/**
 * A helper for registering and working with C++ objects and types in Lua.
 * \tparam T C++ class type
 */
template <typename T>
class Luna {
    typedef struct { T* pT; } Userdata;

public:
    typedef int Index; /**< Lua table index type */

    typedef int (T::*Method)(lua_State* l); /**< Luna method type */

    typedef struct {
        const char* name;
        Method method;
    } MethodMap; /**< Mapping of method name to method */

    /**
     * Commit the class into Lua's global registry.
     * \param l Lua state
     */
    static void commit(lua_State* l) {
        lua_newtable(l);
        int methods = lua_gettop(l);

        luaL_newmetatable(l, T::s_lunaClassName);
        int metatable = lua_gettop(l);

        lua_pushstring(l, T::s_lunaClassName);
        lua_pushvalue(l, methods);
        lua_settable(l, LUA_GLOBALSINDEX);

        lua_pushliteral(l, "__metatable");
        lua_pushvalue(l, methods);
        lua_settable(l, metatable);

        lua_pushliteral(l, "__index");
        lua_pushvalue(l, methods);
        lua_settable(l, metatable);

        lua_pushliteral(l, "__gc");
        lua_pushcfunction(l, gcT);
        lua_settable(l, metatable);

        lua_newtable(l);
        int methodTable = lua_gettop(l);
        lua_pushliteral(l, "__call");
        lua_pushcfunction(l, newT);
        lua_pushliteral(l, "new");
        lua_pushvalue(l, -2);
        lua_settable(l, methods);
        lua_settable(l, methodTable);
        lua_setmetatable(l, methods);

        for (MethodMap* m = T::s_lunaMethods; m->name; ++m) {
            lua_pushstring(l, m->name);
            lua_pushlightuserdata(l, reinterpret_cast<void*>(m));
            lua_pushcclosure(l, thunk, 1);
            lua_settable(l, methods);
        }

        lua_pop(l, 2);
    }

    /**
     * Return the object of type T from the argument narg.
     * \param l Lua state
     * \param narg Argument index
     * \returns Pointer to object or NULL if it isn't of type T
     */
    static T* check(lua_State* l, int narg) {
        Userdata* ud = reinterpret_cast<Userdata*>(luaL_checkudata(l, narg, T::s_lunaClassName));
        if (!ud) {
            luaL_typerror(l, narg, T::s_lunaClassName);
        }
        return ud->pT;
    }

private:
    Luna();

    static int thunk(lua_State* l) {
        T* obj = check(l, 1);
        lua_remove(l, 1);
        MethodMap* m = reinterpret_cast<MethodMap*>(lua_touserdata(l, lua_upvalueindex(1)));
        return (obj->*(m->method))(l);
    }

    static int newT(lua_State* l) {
        lua_remove(l, 1);
        T* obj = new T(l);
        Userdata* ud = reinterpret_cast<Userdata*>(lua_newuserdata(l, sizeof(Userdata)));
        ud->pT = obj;
        luaL_getmetatable(l, T::s_lunaClassName);
        lua_setmetatable(l, -2);
        return 1;
    }

    static int gcT(lua_State* l) {
        Userdata* ud = reinterpret_cast<Userdata*>(lua_touserdata(l, 1));
        T* obj = ud->pT;
        delete obj, obj = NULL;
        return 0;
    }
};

#endif /* LUNA_HPP */

