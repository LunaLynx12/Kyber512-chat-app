#include "lua_integration.h"
#include <lua5.3/lua.hpp>
#include <iostream>

void execute_lua_script(const std::string &script) {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    if (luaL_dofile(L, script.c_str()) != LUA_OK) {
        std::cerr << "Error executing Lua script: " << lua_tostring(L, -1) << std::endl;
    }

    lua_close(L);
}
