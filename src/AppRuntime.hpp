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
    double m_errorDisplayTimer = 0.0;                     // How long to display error message
    static constexpr double ERROR_DISPLAY_DURATION = 5.0; // Show error for 5 seconds
};
