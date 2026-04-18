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

inline int lua_slider(lua_State *L)
{
    EngineAPI *api = GetEngineApi(L);
    const char *id = luaL_checkstring(L, 1);
    float value = static_cast<float>(luaL_checknumber(L, 2));
    float minValue = static_cast<float>(luaL_checknumber(L, 3));
    float maxValue = static_cast<float>(luaL_checknumber(L, 4));
    float x = static_cast<float>(luaL_checknumber(L, 5));
    float y = static_cast<float>(luaL_checknumber(L, 6));
    float w = static_cast<float>(luaL_checknumber(L, 7));
    const char *label = luaL_optstring(L, 8, id);

    const float result = api->Slider(id, value, minValue, maxValue, x, y, w, label);
    lua_pushnumber(L, result);
    return 1;
}

inline int lua_text_field(lua_State *L)
{
    EngineAPI *api = GetEngineApi(L);
    const char *id = luaL_checkstring(L, 1);
    const char *value = luaL_checkstring(L, 2);
    int maxChars = static_cast<int>(luaL_checkinteger(L, 3));
    float x = static_cast<float>(luaL_checknumber(L, 4));
    float y = static_cast<float>(luaL_checknumber(L, 5));
    float w = static_cast<float>(luaL_checknumber(L, 6));
    float h = static_cast<float>(luaL_checknumber(L, 7));
    const char *label = luaL_optstring(L, 8, id);

    const std::string result = api->TextField(id, value, maxChars, x, y, w, h, label);
    lua_pushstring(L, result.c_str());
    return 1;
}

inline int lua_checkbox(lua_State *L)
{
    EngineAPI *api = GetEngineApi(L);
    const char *id = luaL_checkstring(L, 1);
    bool checked = lua_toboolean(L, 2) != 0;
    float x = static_cast<float>(luaL_checknumber(L, 3));
    float y = static_cast<float>(luaL_checknumber(L, 4));
    float size = static_cast<float>(luaL_checknumber(L, 5));
    const char *label = luaL_optstring(L, 6, id);

    bool result = api->Checkbox(id, checked, x, y, size, label);
    lua_pushboolean(L, result ? 1 : 0);
    return 1;
}

inline int lua_main_menu(lua_State *L)
{
    GetEngineApi(L)->MainMenu();
    return 0;
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

inline int lua_run_game(lua_State *L)
{
    GetEngineApi(L)->RunGameFrame();
    return 0;
}

inline int lua_set_spell_config(lua_State *L)
{
    EngineAPI *api = GetEngineApi(L);
    int projectileCount = static_cast<int>(luaL_checkinteger(L, 1));
    float projectileSpeed = static_cast<float>(luaL_checknumber(L, 2));
    float projectileSize = static_cast<float>(luaL_checknumber(L, 3));
    bool ricochet = lua_toboolean(L, 4) != 0;
    int damage = static_cast<int>(luaL_checkinteger(L, 5));

    api->SetSpellConfig(projectileCount, projectileSpeed, projectileSize, ricochet, damage);
    return 0;
}

inline int lua_number_key_pressed(lua_State *L)
{
    lua_pushinteger(L, GetEngineApi(L)->NumberKeyPressed1To5());
    return 1;
}

inline int lua_next_spell_pressed(lua_State *L)
{
    lua_pushboolean(L, GetEngineApi(L)->NextSpellPressed() ? 1 : 0);
    return 1;
}

inline int lua_prev_spell_pressed(lua_State *L)
{
    lua_pushboolean(L, GetEngineApi(L)->PrevSpellPressed() ? 1 : 0);
    return 1;
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

inline void set_engine_function(lua_State *L, const char *name, lua_CFunction fn, EngineAPI &api)
{
    lua_pushlightuserdata(L, &api);
    lua_pushcclosure(L, fn, 1);
    lua_setfield(L, -2, name);
}

inline void set_global_engine_function(lua_State *L, const char *name, lua_CFunction fn, EngineAPI &api)
{
    lua_pushlightuserdata(L, &api);
    lua_pushcclosure(L, fn, 1);
    lua_setglobal(L, name);
}

inline void registerEngineApi(lua_State *L, EngineAPI &api)
{
    lua_newtable(L);

    set_engine_function(L, "button", lua_button, api);
    set_engine_function(L, "text", lua_text, api);
    set_engine_function(L, "slider", lua_slider, api);
    set_engine_function(L, "text_field", lua_text_field, api);
    set_engine_function(L, "checkbox", lua_checkbox, api);
    set_engine_function(L, "main_menu", lua_main_menu, api);
    set_engine_function(L, "start_game", lua_start_game, api);
    set_engine_function(L, "open_editor", lua_open_editor, api);
    set_engine_function(L, "quit_game", lua_quit_game, api);
    set_engine_function(L, "run_game", lua_run_game, api);
    set_engine_function(L, "set_spell_config", lua_set_spell_config, api);
    set_engine_function(L, "number_key_pressed", lua_number_key_pressed, api);
    set_engine_function(L, "next_spell_pressed", lua_next_spell_pressed, api);
    set_engine_function(L, "prev_spell_pressed", lua_prev_spell_pressed, api);
    set_engine_function(L, "screen_width", lua_screen_width, api);
    set_engine_function(L, "screen_height", lua_screen_height, api);
    set_engine_function(L, "delta_time", lua_delta_time, api);

    lua_setglobal(L, "engine");

    // Backward-compatible globals while scripts migrate to engine.* calls.
    set_global_engine_function(L, "button", lua_button, api);
    set_global_engine_function(L, "text", lua_text, api);
    set_global_engine_function(L, "slider", lua_slider, api);
    set_global_engine_function(L, "text_field", lua_text_field, api);
    set_global_engine_function(L, "checkbox", lua_checkbox, api);
    set_global_engine_function(L, "main_menu", lua_main_menu, api);
    set_global_engine_function(L, "start_game", lua_start_game, api);
    set_global_engine_function(L, "open_editor", lua_open_editor, api);
    set_global_engine_function(L, "quit_game", lua_quit_game, api);
    set_global_engine_function(L, "run_game", lua_run_game, api);
    set_global_engine_function(L, "set_spell_config", lua_set_spell_config, api);
    set_global_engine_function(L, "number_key_pressed", lua_number_key_pressed, api);
    set_global_engine_function(L, "next_spell_pressed", lua_next_spell_pressed, api);
    set_global_engine_function(L, "prev_spell_pressed", lua_prev_spell_pressed, api);
    set_global_engine_function(L, "screen_width", lua_screen_width, api);
    set_global_engine_function(L, "screen_height", lua_screen_height, api);
    set_global_engine_function(L, "delta_time", lua_delta_time, api);
}