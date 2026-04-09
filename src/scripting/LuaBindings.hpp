#pragma once
#include "engine/EngineAPI.hpp"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

inline EngineAPI *GetEngineApi(lua_State *L)
{
    return static_cast<EngineAPI *>(lua_touserdata(L, lua_upvalueindex(1)));
}

inline int lua_button(lua_State *L)
{
    EngineAPI *api = GetEngineApi(L);
    const char *id = luaL_checkstring(L, 1);
    float x = static_cast<float>(luaL_checknumber(L, 2));
    float y = static_cast<float>(luaL_checknumber(L, 3));
    float w = static_cast<float>(luaL_checknumber(L, 4));
    float h = static_cast<float>(luaL_checknumber(L, 5));
    const char *text = luaL_checkstring(L, 6);
    bool pressed = api->Button(id, x, y, w, h, text);
    lua_pushboolean(L, pressed ? 1 : 0);
    return 1;
}

inline int lua_text(lua_State *L)
{
    EngineAPI *api = GetEngineApi(L);
    const char *text = luaL_checkstring(L, 1);
    int fontSize = static_cast<int>(luaL_checkinteger(L, 2));
    float x = static_cast<float>(luaL_checknumber(L, 3));
    float y = static_cast<float>(luaL_checknumber(L, 4));
    float w = static_cast<float>(luaL_checknumber(L, 5));
    float h = static_cast<float>(luaL_checknumber(L, 6));
    bool drawn = api->Text(text, fontSize, x, y, w, h);
    lua_pushboolean(L, drawn ? 1 : 0);
    return 1;
}

inline int lua_start_game(lua_State *L)
{
    GetEngineApi(L)->StartGame();
    return 0;
}

inline int lua_open_editor(lua_State *L)
{
    GetEngineApi(L)->OpenEditor();
    return 0;
}

inline int lua_quit_game(lua_State *L)
{
    GetEngineApi(L)->Quit();
    return 0;
}

inline int lua_screen_width(lua_State *L)
{
    lua_pushnumber(L, GetEngineApi(L)->ScreenWidth());
    return 1;
}

inline int lua_screen_height(lua_State *L)
{
    lua_pushnumber(L, GetEngineApi(L)->ScreenHeight());
    return 1;
}

inline int lua_delta_time(lua_State *L)
{
    lua_pushnumber(L, GetEngineApi(L)->DeltaTime());
    return 1;
}

inline void registerEngineApi(lua_State *L, EngineAPI &api)
{
    lua_pushlightuserdata(L, &api);
    lua_pushcclosure(L, lua_button, 1);
    lua_setglobal(L, "button");

    lua_pushlightuserdata(L, &api);
    lua_pushcclosure(L, lua_text, 1);
    lua_setglobal(L, "text");

    lua_pushlightuserdata(L, &api);
    lua_pushcclosure(L, lua_start_game, 1);
    lua_setglobal(L, "start_game");

    lua_pushlightuserdata(L, &api);
    lua_pushcclosure(L, lua_open_editor, 1);
    lua_setglobal(L, "open_editor");

    lua_pushlightuserdata(L, &api);
    lua_pushcclosure(L, lua_quit_game, 1);
    lua_setglobal(L, "quit_game");

    lua_pushlightuserdata(L, &api);
    lua_pushcclosure(L, lua_screen_width, 1);
    lua_setglobal(L, "screen_width");

    lua_pushlightuserdata(L, &api);
    lua_pushcclosure(L, lua_screen_height, 1);
    lua_setglobal(L, "screen_height");

    lua_pushlightuserdata(L, &api);
    lua_pushcclosure(L, lua_delta_time, 1);
    lua_setglobal(L, "delta_time");
}