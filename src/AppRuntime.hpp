#pragma once

#include <string>

#include "engine/EngineAPI.hpp"

class AppRuntime
{
public:
    AppRuntime();

    bool Initialize();
    void Run();
    void Shutdown();

private:
    bool LoadSceneScript(SceneID scene);
    void HandlePendingSceneTransition();
    void RunFrame();

    EngineState m_state{};
    EngineAPI m_api;
    struct lua_State *m_lua = nullptr;
    SceneID m_activeScene = SceneID::MainMenu;
    std::string m_luaRuntimeError;
};
