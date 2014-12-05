#pragma once

/*
** pseudo-indices
*/
#define LUA_REGISTRYINDEX       (-10000)
#define LUA_ENVIRONINDEX        (-10001)
#define LUA_GLOBALSINDEX        (-10002)
#define lua_upvalueindex(i)     (LUA_GLOBALSINDEX-(i))

/* thread status */
#define LUA_OK          0
#define LUA_YIELD       1
#define LUA_ERRRUN      2
#define LUA_ERRSYNTAX   3
#define LUA_ERRMEM      4
#define LUA_ERRGCMM     5
#define LUA_ERRERR      6

#define LUA_TNONE               (-1)

#define LUA_TNIL                0
#define LUA_TBOOLEAN            1
#define LUA_TLIGHTUSERDATA      2
#define LUA_TNUMBER             3
#define LUA_TSTRING             4
#define LUA_TTABLE              5
#define LUA_TFUNCTION           6
#define LUA_TUSERDATA           7

typedef int (*lua_CFunction) (void *L);
typedef struct luaL_Reg {
	const char *name;
	lua_CFunction func; // Maybe array?
	DWORD tempStub;
} luaL_Reg;

typedef int (*luaL_loadbuffer_t)(void* L, const char* buffer, size_t size, const char* name);
typedef int (*lua_pcall_t)(void* L, int nargs, int nresults, int errfunc);
typedef void (*lua_register_t)(void *L, const char *name, const luaL_Reg* l);
typedef int (*lua_gettop_t)(void *L);
typedef void (*lua_settop_t)(void *L, int index);
typedef int (*lua_type_t)(void *L, int idx);
typedef const char* (*lua_tolstring_t)(void *L, int idx, size_t *len);
typedef double (*lua_tonumber_t)(void *L, int index);
typedef void (*lua_pushnumber_t)(void *L, double n);
typedef void (*lua_pushintenger_t)(void *L, int n);
typedef int (*luaL_ref_t)(void *L, int t);
typedef void (*lua_rawgeti_t)(void *L, int idx, int n);
typedef int (*lua_tointeger_t)(void* L, int index);

class LuaFunctions
{
public:
	static void Initialize();

	static luaL_loadbuffer_t pluaL_loadbuffer;
	static lua_pcall_t plua_pcall;
	static lua_register_t lua_register;
	static lua_gettop_t lua_gettop;
	static lua_settop_t lua_settop;
	static lua_type_t lua_type;
	static lua_tolstring_t lua_tolstring;
	static lua_tonumber_t lua_tonumber;	// Appears to be broken
	static lua_pushnumber_t lua_pushnumber; // Appears to be broken
	static lua_pushintenger_t lua_pushintenger;
	static luaL_ref_t luaL_ref;
	static lua_rawgeti_t lua_rawgeti;
	static lua_tointeger_t lua_tointeger;

	#define lua_tostring(L,i)       lua_tolstring(L, (i), NULL)
	#define lua_pop(L,n)  lua_settop(L, -(n)-1)

	int lua_isstring (void *L, int idx) {
	  int t = lua_type(L, idx);
	  return (t == LUA_TSTRING || t == LUA_TNUMBER);
	}
};