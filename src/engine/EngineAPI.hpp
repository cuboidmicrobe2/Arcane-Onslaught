#pragma once

#include <string>
#include <unordered_map>
#include <entt/entt.hpp>
#include <raylib.h>

extern "C"
{
#include <lua.h>
}

enum class SceneID
{
    MainMenu,
    Game,
    Editor,
    Settings,
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

    /// @brief Draws a filled rectangle.
    /// @param x Left position in pixels.
    /// @param y Top position in pixels.
    /// @param w Width in pixels.
    /// @param h Height in pixels.
    /// @param r Red component (0-255).
    /// @param g Green component (0-255).
    /// @param b Blue component (0-255).
    /// @param a Alpha component (0-255).
    void DrawRect(float x, float y, float w, float h, unsigned char r, unsigned char g, unsigned char b, unsigned char a);

    /// @brief Queues transition to the main menu scene.
    void MainMenu();

    /// @brief Queues transition to the gameplay scene.
    void StartGame();

    /// @brief Queues transition to the editor scene.
    void OpenEditor();

    /// @brief Queues transition to the settings scene.
    void OpenSettings();

    /// @brief Requests graceful application shutdown.
    void Quit();

    /// @brief Handles scene-transition side effects when a new scene becomes active.
    /// @param scene The destination scene that was selected.
    void OnSceneChanged(SceneID scene);

    /// @brief Loads the gameplay ECS from the active Lua scene table.
    /// @param L Lua state that currently has the scene table on top of the stack.
    /// @return True when import succeeds.
    bool LoadGameSceneFromLua(struct lua_State *L);

    /// @brief Executes one gameplay frame update and render pass.
    void RunGameFrame();

    /// @brief Updates the active spell tuning values used by gameplay systems.
    /// @param projectileCount Number of projectiles fired.
    /// @param projectileSpeed Projectile movement speed.
    /// @param projectileSize Projectile radius/size.
    /// @param ricochet Whether projectiles bounce on collision.
    /// @param damage Base damage applied on hit.
    /// @param colorR Red component (0.0-1.0).
    /// @param colorG Green component (0.0-1.0).
    /// @param colorB Blue component (0.0-1.0).
    /// @param staggerDelay Delay in seconds between projectile spawns (0 = no stagger).
    void SetSpellConfig(int projectileCount, float projectileSpeed, float projectileSize, bool ricochet, int damage, float colorR = 1.0f, float colorG = 0.82f, float colorB = 0.33f, float staggerDelay = 0.0f);

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

    /// @brief Checks if the window is in fullscreen mode.
    /// @return True if fullscreen is enabled.
    bool IsFullscreen() const;

    /// @brief Toggles fullscreen mode.
    /// @param enable True to enable fullscreen, false for windowed.
    void SetFullscreen(bool enable);

    /// @brief Sets the window resolution.
    /// @param width New width in pixels.
    /// @param height New height in pixels.
    void SetResolution(int width, int height);

    /// @brief Spawns an entity from a Lua table definition (ECS-compliant).
    /// @param L The Lua state. Table at top of stack will be read and instantiated.
    void SpawnEntity(lua_State *L);

    /// @brief Spawns an enemy entity at the specified position and velocity (legacy).
    /// @param x X position in pixels.
    /// @param y Y position in pixels.
    /// @param vx X velocity component.
    /// @param vy Y velocity component.
    /// @param radius Collision radius of the enemy.
    void SpawnEnemy(float x, float y, float vx, float vy, float radius);

    /// @brief Returns the count of active enemy entities.
    /// @return Number of entities with the enemy_tag component.
    int CountEnemies() const;

    /// @brief Returns the active gameplay registry for entity/component composition.
    entt::registry &Registry();
    const entt::registry &Registry() const;

    /// @brief Pushes all entity IDs onto the Lua stack as a table.
    /// @param L The Lua state. A new table of entity IDs will be pushed.
    void GetAllEntities(lua_State *L);

    /// @brief Pushes an entity's component data onto the Lua stack as a table.
    /// @param L The Lua state. Component data table will be pushed, or nil if entity doesn't exist.
    /// @param entityId The entity handle.
    void GetEntityData(lua_State *L, uint32_t entityId);

    /// @brief Updates an entity's components from a Lua table definition.
    /// @param L The Lua state. Table at top of stack contains component updates.
    /// @param entityId The entity handle to update.
    /// @return True if the entity exists and update succeeds.
    bool UpdateEntityData(lua_State *L, uint32_t entityId);

    /// @brief Removes an entity from the registry.
    /// @param entityId The entity handle to delete.
    void DeleteEntity(uint32_t entityId);

    /// @brief Attaches a behavior co-routine to an entity.
    /// @param entityId The entity handle.
    /// @param L The Lua state. Co-routine at top of stack.
    void AttachBehavior(uint32_t entityId, lua_State *L);

private:
    void ResetGameEcs();

    EngineState &m_state;
    entt::registry m_registry;
    bool m_gameInitialized = false;
    struct lua_State *m_lua = nullptr;
    std::unordered_map<std::string, bool> m_prevDown;
    std::string m_activeSliderId;
    std::string m_activeTextFieldId;
};