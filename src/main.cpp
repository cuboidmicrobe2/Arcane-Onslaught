#include <iostream>
#include <raylib.h>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "engine/EngineAPI.hpp"
#include "scripting/LuaBindings.hpp"

int main()
{
    InitWindow(1280, 720, "Arcane Onslaught");
    SetTargetFPS(60);

    EngineState state;
    EngineAPI api(state);

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    registerEngineApi(L, api);

    if (luaL_dofile(L, "scripts/main_menu.lua") != LUA_OK)
    {
        std::cerr << "Lua load error: " << lua_tostring(L, -1) << "\n";
        lua_pop(L, 1);
    }

    while (!WindowShouldClose() && !state.shouldQuit)
    {
        BeginDrawing();
        ClearBackground(Color{24, 24, 30, 255});

        api.BeginFrame();

        lua_getglobal(L, "update_menu");
        if (lua_isfunction(L, -1))
        {
            if (lua_pcall(L, 0, 0, 0) != LUA_OK)
            {
                const char *err = lua_tostring(L, -1);
                if (err != nullptr)
                {
                    DrawText(err, 20, 20, 20, RED);
                }
                lua_pop(L, 1);
            }
        }
        else
        {
            lua_pop(L, 1);
        }

        api.EndFrame();
        EndDrawing();

        if (state.nextScene == SceneID::Game)
        {
            // switch to game scene
        }
        else if (state.nextScene == SceneID::Editor)
        {
            // switch to editor scene
        }
    }

    lua_close(L);
    CloseWindow();
    return 0;
}