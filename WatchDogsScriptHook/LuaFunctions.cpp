#include "stdafx.h"
#include "LuaFunctions.h"
#include "WDOffsets.h"

luaL_loadbuffer_t LuaFunctions::pluaL_loadbuffer = NULL;
lua_pcall_t LuaFunctions::plua_pcall = NULL;
lua_register_t LuaFunctions::lua_register = NULL;
lua_gettop_t LuaFunctions::lua_gettop = NULL;
lua_settop_t LuaFunctions::lua_settop = NULL;
lua_type_t LuaFunctions::lua_type = NULL;
lua_tolstring_t LuaFunctions::lua_tolstring = NULL;
lua_tonumber_t LuaFunctions::lua_tonumber = NULL;
lua_pushnumber_t LuaFunctions::lua_pushnumber = NULL;
lua_pushintenger_t LuaFunctions::lua_pushintenger = NULL;
luaL_ref_t LuaFunctions::luaL_ref = NULL;
lua_rawgeti_t LuaFunctions::lua_rawgeti = NULL;
lua_tointeger_t LuaFunctions::lua_tointeger = NULL;

void LuaFunctions::Initialize()
{
	pluaL_loadbuffer = (luaL_loadbuffer_t) ((DWORD64)(WDOffsets::GetGameBase() + 0x17FBD70));
    plua_pcall = (lua_pcall_t) ((DWORD64)(WDOffsets::GetGameBase() + 0x17C7270));
	lua_register = (lua_register_t) ((DWORD64)(WDOffsets::GetGameBase() + 0x17FCB20));
	lua_gettop = (lua_gettop_t) ((DWORD64)(WDOffsets::GetGameBase() + 0x17C6190));
	lua_settop = (lua_settop_t) ((DWORD64)(WDOffsets::GetGameBase() + 0x17C61B0));
	lua_type = (lua_type_t) ((DWORD64)(WDOffsets::GetGameBase() + 0x17C63C0));
	lua_tolstring = (lua_tolstring_t) ((DWORD64)(WDOffsets::GetGameBase() + 0x17C66E0));
	lua_tonumber = (lua_tonumber_t) ((DWORD64)(WDOffsets::GetGameBase() + 0x17C6620));
	lua_pushnumber = (lua_pushnumber_t) ((DWORD64)(WDOffsets::GetGameBase() + 0x17C6950));
	lua_pushintenger = (lua_pushintenger_t) ((DWORD64)(WDOffsets::GetGameBase() + 0x17C6970));
	luaL_ref = (luaL_ref_t) ((DWORD64)(WDOffsets::GetGameBase() + 0x17FB8B0));
	lua_rawgeti = (lua_rawgeti_t) ((DWORD64)(WDOffsets::GetGameBase() + 0x17C6D40));
	lua_tointeger = (lua_tointeger_t) ((DWORD64)(WDOffsets::GetGameBase() + 0x17C6D40));
}