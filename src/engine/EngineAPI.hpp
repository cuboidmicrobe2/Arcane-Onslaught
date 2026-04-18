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

    /// @brief Begins per-frame engine state updates before scene logic runs.
    void BeginFrame();

    /// @brief Ends per-frame engine state updates after scene logic runs.
    void EndFrame();

    /// @brief Draws a clickable button.
    /// @param id Unique widget id used to keep interaction state stable across frames.
    /// @param x Left position in pixels.
    /// @param y Top position in pixels.
    /// @param w Width in pixels.
    /// @param h Height in pixels.
    /// @param text Label rendered inside the button.
    /// @return True on the frame the button is pressed, false otherwise.
    bool Button(const std::string &id, float x, float y, float w, float h, const std::string &text);

    /// @brief Draws text inside a rectangular region.
    /// @param text String to render.
    /// @param fontSize Font size in pixels.
    /// @param x Left position in pixels.
    /// @param y Top position in pixels.
    /// @param w Bounds width in pixels.
    /// @param h Bounds height in pixels.
    /// @return True when draw call is accepted.
    bool Text(const std::string &text, int fontSize, float x, float y, float w, float h);

    /// @brief Draws a horizontal slider and returns the updated value.
    /// @param id Unique widget id used to keep drag state stable across frames.
    /// @param value Current slider value.
    /// @param minValue Minimum value allowed.
    /// @param maxValue Maximum value allowed.
    /// @param x Left position in pixels.
    /// @param y Top position in pixels.
    /// @param w Slider width in pixels.
    /// @param label Text shown next to the slider.
    /// @return Updated slider value for the current frame.
    float Slider(const std::string &id, float value, float minValue, float maxValue, float x, float y, float w, const std::string &label);

    /// @brief Draws a checkbox and returns its current checked state.
    /// @param id Unique widget id used to keep interaction state stable across frames.
    /// @param checked Current checked value.
    /// @param x Left position in pixels.
    /// @param y Top position in pixels.
    /// @param size Checkbox side length in pixels.
    /// @param label Text shown next to the checkbox.
    /// @return Updated checked value.
    bool Checkbox(const std::string &id, bool checked, float x, float y, float size, const std::string &label);

    /// @brief Draws an editable single-line text field.
    /// @param id Unique widget id used to keep focus and edit state across frames.
    /// @param value Current text value.
    /// @param maxChars Maximum number of accepted characters.
    /// @param x Left position in pixels.
    /// @param y Top position in pixels.
    /// @param w Field width in pixels.
    /// @param h Field height in pixels.
    /// @param label Text shown as field label.
    /// @return Updated text value after this frame.
    std::string TextField(const std::string &id, const std::string &value, int maxChars, float x, float y, float w, float h, const std::string &label);

    /// @brief Queues transition to the main menu scene.
    void MainMenu();

    /// @brief Queues transition to the gameplay scene.
    void StartGame();

    /// @brief Queues transition to the editor scene.
    void OpenEditor();

    /// @brief Requests graceful application shutdown.
    void Quit();

    /// @brief Handles scene-transition side effects when a new scene becomes active.
    /// @param scene The destination scene that was selected.
    void OnSceneChanged(SceneID scene);

    /// @brief Executes one gameplay frame update and render pass.
    void RunGameFrame();

    /// @brief Updates the active spell tuning values used by gameplay systems.
    /// @param projectileCount Number of projectiles fired.
    /// @param projectileSpeed Projectile movement speed.
    /// @param projectileSize Projectile radius/size.
    /// @param ricochet Whether projectiles bounce on collision.
    /// @param damage Base damage applied on hit.
    void SetSpellConfig(int projectileCount, float projectileSpeed, float projectileSize, bool ricochet, int damage);

    /// @brief Reports which numeric spell-slot key (1-5) was pressed this frame.
    /// @return Value in [1, 5], or 0 when no slot key was pressed.
    int NumberKeyPressed1To5() const;

    /// @brief Checks whether next-spell input was triggered this frame.
    /// @return True when next-spell action is pressed.
    bool NextSpellPressed() const;

    /// @brief Checks whether previous-spell input was triggered this frame.
    /// @return True when previous-spell action is pressed.
    bool PrevSpellPressed() const;

    /// @brief Returns current screen width in pixels.
    float ScreenWidth() const;

    /// @brief Returns current screen height in pixels.
    float ScreenHeight() const;

    /// @brief Returns frame delta time in seconds.
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