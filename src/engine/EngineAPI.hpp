#pragma once

#include <string>
#include <unordered_map>
#include <entt/entt.hpp>
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
    float Slider(const std::string &id, float value, float minValue, float maxValue, float x, float y, float w, const std::string &label);
    bool Checkbox(const std::string &id, bool checked, float x, float y, float size, const std::string &label);
    std::string TextField(const std::string &id, const std::string &value, int maxChars, float x, float y, float w, float h, const std::string &label);

    void MainMenu();
    void StartGame();
    void OpenEditor();
    void Quit();
    void OnSceneChanged(SceneID scene);

    void RunGameFrame();
    void SetSpellConfig(int projectileCount, float projectileSpeed, float projectileSize, bool ricochet, int damage);
    int NumberKeyPressed1To5() const;
    bool NextSpellPressed() const;
    bool PrevSpellPressed() const;

    float ScreenWidth() const;
    float ScreenHeight() const;
    float DeltaTime() const;

private:
    void InitializeGameEcs();

    EngineState &m_state;
    entt::registry m_registry;
    bool m_gameInitialized = false;
    std::unordered_map<std::string, bool> m_prevDown;
    std::string m_activeSliderId;
    std::string m_activeTextFieldId;
};