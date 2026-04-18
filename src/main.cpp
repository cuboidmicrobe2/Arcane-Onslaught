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

    if (luaL_dofile(L, "scripts/spell_config.lua") != LUA_OK)
    {
        std::cerr << "Lua load error (scripts/spell_config.lua): " << lua_tostring(L, -1) << "\n";
        lua_pop(L, 1);
    }

    auto loadSceneScript = [L](SceneID scene) -> bool
    {
        const char *scriptPath = nullptr;
        switch (scene)
        {
        case SceneID::MainMenu:
            scriptPath = "scripts/main_menu.lua";
            break;
        case SceneID::Game:
            scriptPath = "scripts/game.lua";
            break;
        case SceneID::Editor:
            scriptPath = "scripts/editor.lua";
            break;
        case SceneID::Quit:
            return true;
        }

        if (luaL_dofile(L, scriptPath) != LUA_OK)
        {
            std::cerr << "Lua load error (" << scriptPath << "): " << lua_tostring(L, -1) << "\n";
            lua_pop(L, 1);
            return false;
        }

        return true;
    };

    auto getUpdateFunctionName = [](SceneID scene) -> const char *
    {
        switch (scene)
        {
        case SceneID::MainMenu:
            return "update_menu";
        case SceneID::Game:
            return "update_game";
        case SceneID::Editor:
            return "update_editor";
        case SceneID::Quit:
            return nullptr;
        }

        return nullptr;
    };

    SceneID activeScene = state.nextScene;
    if (!loadSceneScript(activeScene))
    {
        state.shouldQuit = true;
    }
    else
    {
        api.OnSceneChanged(activeScene);
    }

    while (!WindowShouldClose() && !state.shouldQuit)
    {
        if (state.nextScene != activeScene)
        {
            activeScene = state.nextScene;
            if (!loadSceneScript(activeScene))
            {
                state.shouldQuit = true;
                continue;
            }

            api.OnSceneChanged(activeScene);
        }

        BeginDrawing();
        ClearBackground(Color{24, 24, 30, 255});

        api.BeginFrame();

        const char *updateFunction = getUpdateFunctionName(activeScene);
        if (updateFunction != nullptr)
        {
            lua_getglobal(L, updateFunction);
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
        }

        api.EndFrame();
        EndDrawing();
    }

    lua_close(L);
    CloseWindow();
    return 0;
}