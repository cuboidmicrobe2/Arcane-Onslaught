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

inline int lua_draw_rect(lua_State *L)
{
    EngineAPI *api = GetEngineApi(L);
    float x = static_cast<float>(luaL_checknumber(L, 1));
    float y = static_cast<float>(luaL_checknumber(L, 2));
    float w = static_cast<float>(luaL_checknumber(L, 3));
    float h = static_cast<float>(luaL_checknumber(L, 4));
    unsigned char r = static_cast<unsigned char>(luaL_checkinteger(L, 5));
    unsigned char g = static_cast<unsigned char>(luaL_checkinteger(L, 6));
    unsigned char b = static_cast<unsigned char>(luaL_checkinteger(L, 7));
    unsigned char a = static_cast<unsigned char>(luaL_checkinteger(L, 8));

    api->DrawRect(x, y, w, h, r, g, b, a);
    return 0;
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

inline int lua_open_settings(lua_State *L)
{
    GetEngineApi(L)->OpenSettings();
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

    // Color parameters (optional, default to golden yellow if not provided)
    float r = 1.0f, g = 0.82f, b = 0.33f;
    if (lua_isnumber(L, 6) && lua_isnumber(L, 7) && lua_isnumber(L, 8))
    {
        r = static_cast<float>(luaL_checknumber(L, 6));
        g = static_cast<float>(luaL_checknumber(L, 7));
        b = static_cast<float>(luaL_checknumber(L, 8));
    }

    // Stagger delay (optional, default to 0.0 if not provided)
    float staggerDelay = 0.0f;
    if (lua_isnumber(L, 9))
    {
        staggerDelay = static_cast<float>(luaL_checknumber(L, 9));
    }

    api->SetSpellConfig(projectileCount, projectileSpeed, projectileSize, ricochet, damage, r, g, b, staggerDelay);
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

inline int lua_is_fullscreen(lua_State *L)
{
    lua_pushboolean(L, GetEngineApi(L)->IsFullscreen() ? 1 : 0);
    return 1;
}

inline int lua_set_fullscreen(lua_State *L)
{
    bool enable = lua_toboolean(L, 1) != 0;
    GetEngineApi(L)->SetFullscreen(enable);
    return 0;
}

inline int lua_set_resolution(lua_State *L)
{
    int width = static_cast<int>(luaL_checkinteger(L, 1));
    int height = static_cast<int>(luaL_checkinteger(L, 2));
    GetEngineApi(L)->SetResolution(width, height);
    return 0;
}

inline int lua_spawn_enemy(lua_State *L)
{
    EngineAPI *api = GetEngineApi(L);
    float x = static_cast<float>(luaL_checknumber(L, 1));
    float y = static_cast<float>(luaL_checknumber(L, 2));
    float vx = static_cast<float>(luaL_checknumber(L, 3));
    float vy = static_cast<float>(luaL_checknumber(L, 4));
    float radius = static_cast<float>(luaL_checknumber(L, 5));

    api->SpawnEnemy(x, y, vx, vy, radius);
    return 0;
}

inline int lua_spawn_entity(lua_State *L)
{
    EngineAPI *api = GetEngineApi(L);
    api->SpawnEntity(L);
    return 1;
}

inline int lua_count_enemies(lua_State *L)
{
    lua_pushinteger(L, GetEngineApi(L)->CountEnemies());
    return 1;
}

inline int lua_get_all_entities(lua_State *L)
{
    GetEngineApi(L)->GetAllEntities(L);
    return 1;
}

inline int lua_get_entity_data(lua_State *L)
{
    EngineAPI *api = GetEngineApi(L);
    uint32_t entityId = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    api->GetEntityData(L, entityId);
    return 1;
}

inline int lua_update_entity_data(lua_State *L)
{
    EngineAPI *api = GetEngineApi(L);
    uint32_t entityId = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    luaL_checktype(L, 2, LUA_TTABLE);
    lua_pushvalue(L, 2);
    bool success = api->UpdateEntityData(L, entityId);
    lua_pop(L, 1);
    lua_pushboolean(L, success ? 1 : 0);
    return 1;
}

inline int lua_delete_entity(lua_State *L)
{
    EngineAPI *api = GetEngineApi(L);
    uint32_t entityId = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    api->DeleteEntity(entityId);
    return 0;
}

inline int lua_attach_behavior(lua_State *L)
{
    EngineAPI *api = GetEngineApi(L);
    uint32_t entityId = static_cast<uint32_t>(luaL_checkinteger(L, 1));
    luaL_checktype(L, 2, LUA_TTHREAD);
    api->AttachBehavior(entityId, L);
    return 0;
}

inline int lua_coroutine_yield_frame(lua_State *L)
{
    // Yield the co-routine - when called from within a behavior, this pauses execution
    return lua_yield(L, 0);
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
    set_engine_function(L, "draw_rect", lua_draw_rect, api);
    set_engine_function(L, "main_menu", lua_main_menu, api);
    set_engine_function(L, "start_game", lua_start_game, api);
    set_engine_function(L, "open_editor", lua_open_editor, api);
    set_engine_function(L, "open_settings", lua_open_settings, api);
    set_engine_function(L, "quit_game", lua_quit_game, api);
    set_engine_function(L, "run_game", lua_run_game, api);
    set_engine_function(L, "set_spell_config", lua_set_spell_config, api);
    set_engine_function(L, "number_key_pressed", lua_number_key_pressed, api);
    set_engine_function(L, "next_spell_pressed", lua_next_spell_pressed, api);
    set_engine_function(L, "prev_spell_pressed", lua_prev_spell_pressed, api);
    set_engine_function(L, "screen_width", lua_screen_width, api);
    set_engine_function(L, "screen_height", lua_screen_height, api);
    set_engine_function(L, "delta_time", lua_delta_time, api);
    set_engine_function(L, "is_fullscreen", lua_is_fullscreen, api);
    set_engine_function(L, "set_fullscreen", lua_set_fullscreen, api);
    set_engine_function(L, "set_resolution", lua_set_resolution, api);
    set_engine_function(L, "spawn_enemy", lua_spawn_enemy, api);
    set_engine_function(L, "spawn_entity", lua_spawn_entity, api);
    set_engine_function(L, "count_enemies", lua_count_enemies, api);
    set_engine_function(L, "get_all_entities", lua_get_all_entities, api);
    set_engine_function(L, "get_entity_data", lua_get_entity_data, api);
    set_engine_function(L, "update_entity_data", lua_update_entity_data, api);
    set_engine_function(L, "delete_entity", lua_delete_entity, api);
    set_engine_function(L, "attach_behavior", lua_attach_behavior, api);
    set_engine_function(L, "coroutine_yield_frame", lua_coroutine_yield_frame, api);

    lua_setglobal(L, "engine");

    // Backward-compatible globals while scripts migrate to engine.* calls.
    set_global_engine_function(L, "button", lua_button, api);
    set_global_engine_function(L, "text", lua_text, api);
    set_global_engine_function(L, "slider", lua_slider, api);
    set_global_engine_function(L, "text_field", lua_text_field, api);
    set_global_engine_function(L, "checkbox", lua_checkbox, api);
    set_global_engine_function(L, "draw_rect", lua_draw_rect, api);
    set_global_engine_function(L, "main_menu", lua_main_menu, api);
    set_global_engine_function(L, "start_game", lua_start_game, api);
    set_global_engine_function(L, "open_editor", lua_open_editor, api);
    set_global_engine_function(L, "open_settings", lua_open_settings, api);
    set_global_engine_function(L, "quit_game", lua_quit_game, api);
    set_global_engine_function(L, "run_game", lua_run_game, api);
    set_global_engine_function(L, "set_spell_config", lua_set_spell_config, api);
    set_global_engine_function(L, "number_key_pressed", lua_number_key_pressed, api);
    set_global_engine_function(L, "next_spell_pressed", lua_next_spell_pressed, api);
    set_global_engine_function(L, "prev_spell_pressed", lua_prev_spell_pressed, api);
    set_global_engine_function(L, "screen_width", lua_screen_width, api);
    set_global_engine_function(L, "screen_height", lua_screen_height, api);
    set_global_engine_function(L, "delta_time", lua_delta_time, api);
    set_global_engine_function(L, "is_fullscreen", lua_is_fullscreen, api);
    set_global_engine_function(L, "set_fullscreen", lua_set_fullscreen, api);
    set_global_engine_function(L, "set_resolution", lua_set_resolution, api);
    set_global_engine_function(L, "spawn_enemy", lua_spawn_enemy, api);
    set_global_engine_function(L, "spawn_entity", lua_spawn_entity, api);
    set_global_engine_function(L, "count_enemies", lua_count_enemies, api);
    set_global_engine_function(L, "get_all_entities", lua_get_all_entities, api);
    set_global_engine_function(L, "get_entity_data", lua_get_entity_data, api);
    set_global_engine_function(L, "update_entity_data", lua_update_entity_data, api);
    set_global_engine_function(L, "delete_entity", lua_delete_entity, api);
    set_global_engine_function(L, "attach_behavior", lua_attach_behavior, api);
    set_global_engine_function(L, "coroutine_yield_frame", lua_coroutine_yield_frame, api);
}