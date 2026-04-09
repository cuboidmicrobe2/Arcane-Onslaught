#pragma once

#include <string>
#include <unordered_map>
#include <raylib.h>

enum class SceneID
{
    MainMenu,
    Game,
    Editor,
    Quit
};

struct EngineState
{
    SceneID nextScene = SceneID::MainMenu;
    bool shouldQuit = false;
};

class EngineAPI
{
public:
    explicit EngineAPI(EngineState &state);

    void BeginFrame();
    void EndFrame();

    bool Button(const std::string &id, float x, float y, float w, float h, const std::string &text);
    bool Text(const std::string &text, int fontSize, float x, float y, float w, float h);

    void StartGame();
    void OpenEditor();
    void Quit();

    float ScreenWidth() const;
    float ScreenHeight() const;
    float DeltaTime() const;

private:
    EngineState &m_state;
    std::unordered_map<std::string, bool> m_prevDown;
};