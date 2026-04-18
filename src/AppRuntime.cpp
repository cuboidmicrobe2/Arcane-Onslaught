#include "AppRuntime.hpp"

#include <iostream>
#include <string>

#include <raylib.h>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "engine/EngineAPI.hpp"
#include "scripting/LuaBindings.hpp"

namespace
{
    const char *GetSceneScriptPath(SceneID scene)
    {
        switch (scene)
        {
        case SceneID::MainMenu:
            return "scripts/main_menu.lua";
        case SceneID::Game:
            return "scripts/game.lua";
        case SceneID::Editor:
            return "scripts/editor.lua";
        case SceneID::Quit:
            return nullptr;
        }

        return nullptr;
    }

    const char *GetSceneUpdateFunctionName(SceneID scene)
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
    }

    bool RunLuaFile(lua_State *L, const char *scriptPath)
    {
        if (luaL_dofile(L, scriptPath) != LUA_OK)
        {
            std::cerr << "Lua load error (" << scriptPath << "): " << lua_tostring(L, -1) << "\n";
            lua_pop(L, 1);
            return false;
        }

        return true;
    }

    bool CallLuaUpdate(lua_State *L, const char *functionName, std::string &errorMessage)
    {
        lua_getglobal(L, functionName);
        if (!lua_isfunction(L, -1))
        {
            lua_pop(L, 1);
            errorMessage = std::string("Missing Lua function: ") + functionName;
            return false;
        }

        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
        {
            const char *err = lua_tostring(L, -1);
            errorMessage = (err != nullptr) ? err : "Unknown Lua runtime error";
            lua_pop(L, 1);
            return false;
        }

        errorMessage.clear();
        return true;
    }
}

AppRuntime::AppRuntime() : m_api(m_state) {}

bool AppRuntime::Initialize()
{
    m_lua = luaL_newstate();
    if (m_lua == nullptr)
    {
        std::cerr << "Failed to create Lua state\n";
        return false;
    }

    luaL_openlibs(m_lua);
    registerEngineApi(m_lua, m_api);

    if (!RunLuaFile(m_lua, "scripts/spell_config.lua"))
    {
        return false;
    }

    m_activeScene = m_state.nextScene;
    if (!LoadSceneScript(m_activeScene))
    {
        return false;
    }

    m_api.OnSceneChanged(m_activeScene);
    return true;
}

void AppRuntime::Run()
{
    while (!WindowShouldClose() && !m_state.shouldQuit)
    {
        RunFrame();
    }
}

void AppRuntime::Shutdown()
{
    if (m_lua != nullptr)
    {
        lua_close(m_lua);
        m_lua = nullptr;
    }
}

bool AppRuntime::LoadSceneScript(SceneID scene)
{
    const char *scriptPath = GetSceneScriptPath(scene);
    if (scriptPath == nullptr)
    {
        return scene == SceneID::Quit;
    }

    return RunLuaFile(m_lua, scriptPath);
}

void AppRuntime::HandlePendingSceneTransition()
{
    if (m_state.nextScene == m_activeScene)
    {
        return;
    }

    m_activeScene = m_state.nextScene;
    m_luaRuntimeError.clear();
    if (!LoadSceneScript(m_activeScene))
    {
        m_state.shouldQuit = true;
        return;
    }

    m_api.OnSceneChanged(m_activeScene);
}

void AppRuntime::RunFrame()
{
    if (IsKeyPressed(KEY_ESCAPE) && m_activeScene != SceneID::MainMenu)
    {
        m_api.MainMenu();
    }

    HandlePendingSceneTransition();
    if (m_state.shouldQuit)
    {
        return;
    }

    BeginDrawing();
    ClearBackground(Color{24, 24, 30, 255});

    m_api.BeginFrame();

    const char *updateFunction = GetSceneUpdateFunctionName(m_activeScene);
    if (updateFunction != nullptr)
    {
        CallLuaUpdate(m_lua, updateFunction, m_luaRuntimeError);
    }

    if (!m_luaRuntimeError.empty())
    {
        DrawText(m_luaRuntimeError.c_str(), 20, 20, 20, RED);
    }

    m_api.EndFrame();
    EndDrawing();
}
