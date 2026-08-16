#include "engine/EngineAPI.hpp"
#include "EngineAPI.hpp"
#include "engine/GameEcs.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

EngineAPI::EngineAPI(EngineState &state) : m_state(state) {}

void EngineAPI::BeginFrame()
{
}

void EngineAPI::EndFrame()
{
}

bool EngineAPI::Button(const std::string &id, float x, float y, float w, float h, const std::string &text)
{
    Rectangle rec = {x, y, w, h};
    Vector2 mousePos = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mousePos, rec);
    bool down = hovered && IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    bool pressed = hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    Color fill = hovered ? Color{80, 80, 96, 255} : Color{55, 55, 70, 255};
    DrawRectangleRounded(rec, 0.2f, 6, fill);
    DrawRectangleRoundedLinesEx(rec, 0.2f, 6, 2.0f, Color{180, 180, 200, 255});

    int fontSize = 22;
    int textWidth = MeasureText(text.c_str(), fontSize);
    DrawText(text.c_str(), static_cast<int>(x + (w - textWidth) * 0.5f), static_cast<int>(y + (h - fontSize) * 0.5f), fontSize, RAYWHITE);

    m_prevDown[id] = down;
    return pressed;
}

bool EngineAPI::Text(const std::string &text, int fontSize, float x, float y, float w, float h)
{
    int textWidth = MeasureText(text.c_str(), fontSize);
    DrawText(text.c_str(), static_cast<int>(x + (w - textWidth) * 0.5f), static_cast<int>(y + (h - fontSize) * 0.5f), fontSize, RAYWHITE);
    return true;
}

float EngineAPI::Slider(const std::string &id, float value, float minValue, float maxValue, float x, float y, float w, const std::string &label)
{
    if (maxValue <= minValue)
    {
        return minValue;
    }

    const float rowHeight = 34.0f;
    const float lineY = y + 22.0f;
    const float lineThickness = 4.0f;
    const float knobRadius = 9.0f;

    Rectangle interactionRect{x, lineY - knobRadius * 2.0f - 10.0f, w, knobRadius * 4.0f};
    Vector2 mousePos = GetMousePosition();
    const bool hovered = CheckCollisionPointRec(mousePos, interactionRect);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hovered)
    {
        m_activeSliderId = id;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && m_activeSliderId == id)
    {
        m_activeSliderId.clear();
    }

    float outputValue = std::clamp(value, minValue, maxValue);
    const bool integerRange =
        std::fabs(minValue - std::round(minValue)) < 0.0001f &&
        std::fabs(maxValue - std::round(maxValue)) < 0.0001f;

    const bool active = (m_activeSliderId == id);
    if (active && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        const float t = std::clamp((mousePos.x - x) / w, 0.0f, 1.0f);
        outputValue = minValue + (maxValue - minValue) * t;
        if (integerRange)
        {
            outputValue = std::round(outputValue);
        }
    }

    outputValue = std::clamp(outputValue, minValue, maxValue);

    const float normalized = (outputValue - minValue) / (maxValue - minValue);
    const float knobX = x + (w * std::clamp(normalized, 0.0f, 1.0f));

    DrawLineEx(Vector2{x, lineY}, Vector2{x + w, lineY}, lineThickness, Color{64, 64, 78, 255});
    DrawLineEx(Vector2{x, lineY}, Vector2{knobX, lineY}, lineThickness, Color{88, 164, 255, 255});
    DrawCircleV(Vector2{knobX, lineY}, knobRadius, active ? Color{255, 218, 122, 255} : Color{220, 230, 255, 255});
    DrawCircleLines(static_cast<int>(knobX), static_cast<int>(lineY), knobRadius, Color{30, 30, 36, 255});

    char valueBuffer[64] = {};
    if (integerRange)
    {
        std::snprintf(valueBuffer, sizeof(valueBuffer), "%d", static_cast<int>(std::round(outputValue)));
    }
    else
    {
        std::snprintf(valueBuffer, sizeof(valueBuffer), "%.2f", outputValue);
    }
    DrawText(label.c_str(), static_cast<int>(x), static_cast<int>(y), 20, RAYWHITE);
    DrawText(valueBuffer, static_cast<int>(x + w + 14.0f), static_cast<int>(y), 20, Color{180, 225, 255, 255});

    return outputValue;
}

bool EngineAPI::Checkbox(const std::string &id, bool checked, float x, float y, float size, const std::string &label)
{
    Rectangle box{x, y, size, size};
    Rectangle hit{x, y, size + 320.0f, size};
    Vector2 mousePos = GetMousePosition();
    const bool hovered = CheckCollisionPointRec(mousePos, hit);

    bool output = checked;
    if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        output = !output;
    }

    const Color fill = hovered ? Color{68, 70, 86, 255} : Color{50, 52, 66, 255};
    const Color border = output ? Color{105, 195, 255, 255} : Color{130, 136, 160, 255};
    DrawRectangleRounded(box, 0.2f, 4, fill);
    DrawRectangleRoundedLinesEx(box, 0.2f, 4, 2.0f, border);

    if (output)
    {
        DrawLineEx(Vector2{x + 5.0f, y + size * 0.56f}, Vector2{x + size * 0.42f, y + size - 6.0f}, 3.0f, Color{180, 235, 255, 255});
        DrawLineEx(Vector2{x + size * 0.42f, y + size - 6.0f}, Vector2{x + size - 5.0f, y + 6.0f}, 3.0f, Color{180, 235, 255, 255});
    }

    DrawText(label.c_str(), static_cast<int>(x + size + 12.0f), static_cast<int>(y + 2.0f), 22, RAYWHITE);
    m_prevDown[id] = hovered && IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    return output;
}

std::string EngineAPI::TextField(const std::string &id,
                                 const std::string &value,
                                 int maxChars,
                                 float x,
                                 float y,
                                 float w,
                                 float h,
                                 const std::string &label)
{
    const int safeMaxChars = std::max(1, maxChars);
    std::string output = value;
    if (static_cast<int>(output.size()) > safeMaxChars)
    {
        output.resize(static_cast<std::size_t>(safeMaxChars));
    }

    const float labelY = y - 24.0f;
    DrawText(label.c_str(), static_cast<int>(x), static_cast<int>(labelY), 20, RAYWHITE);

    Rectangle rec{x, y, w, h};
    const Vector2 mousePos = GetMousePosition();
    const bool hovered = CheckCollisionPointRec(mousePos, rec);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (hovered)
        {
            m_activeTextFieldId = id;
        }
        else if (m_activeTextFieldId == id)
        {
            m_activeTextFieldId.clear();
        }
    }

    const bool active = (m_activeTextFieldId == id);
    if (active)
    {
        int key = GetCharPressed();
        while (key > 0)
        {
            if (key >= 32 && key <= 126 && static_cast<int>(output.size()) < safeMaxChars)
            {
                output.push_back(static_cast<char>(key));
            }

            key = GetCharPressed();
        }

        if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && !output.empty())
        {
            output.pop_back();
        }
    }

    const Color fill = active ? Color{60, 64, 82, 255} : Color{44, 46, 58, 255};
    const Color border = active ? Color{120, 190, 255, 255} : Color{110, 116, 140, 255};
    DrawRectangleRounded(rec, 0.16f, 5, fill);
    DrawRectangleRoundedLinesEx(rec, 0.16f, 5, 2.0f, border);

    std::string visible = output;
    if (active && ((GetTime() - std::floor(GetTime())) < 0.5))
    {
        visible += "_";
    }

    DrawText(visible.c_str(), static_cast<int>(x + 10.0f), static_cast<int>(y + (h - 22.0f) * 0.5f), 22, RAYWHITE);
    return output;
}

void EngineAPI::DrawRect(float x, float y, float w, float h, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    DrawRectangle(
        static_cast<int>(x),
        static_cast<int>(y),
        static_cast<int>(w),
        static_cast<int>(h),
        Color{r, g, b, a});
}

void EngineAPI::MainMenu()
{
    m_state.nextScene = SceneID::MainMenu;
}

void EngineAPI::StartGame()
{
    m_state.nextScene = SceneID::Game;
}

void EngineAPI::OpenEditor()
{
    m_state.nextScene = SceneID::Editor;
}

void EngineAPI::OpenSettings()
{
    m_state.nextScene = SceneID::Settings;
}

void EngineAPI::Quit()
{
    m_state.nextScene = SceneID::Quit;
    m_state.shouldQuit = true;
}

void EngineAPI::OnSceneChanged(SceneID scene)
{
    if (scene != SceneID::Game)
    {
        m_gameInitialized = false;
    }
}

void EngineAPI::ResetGameEcs()
{
    m_registry.clear();
    m_gameInitialized = false;
}

bool EngineAPI::LoadGameSceneFromLua(lua_State *L)
{
    ResetGameEcs();
    m_lua = L;

    if (!GameEcs::InitializeWorld(m_registry, L))
    {
        return false;
    }

    m_gameInitialized = true;
    return true;
}

void EngineAPI::RunGameFrame()
{
    if (!m_gameInitialized)
    {
        return;
    }

    const GameEcs::FrameContext frame{DeltaTime(), ScreenWidth(), ScreenHeight()};

    GameEcs::UpdateBehaviors(m_registry, m_lua, frame);
    GameEcs::UpdatePlayers(m_registry, frame);
    GameEcs::SpawnPlayerProjectiles(m_registry, frame);
    GameEcs::UpdateProjectiles(m_registry, frame);
    GameEcs::ResolveProjectileEnemyCollisions(m_registry);
    GameEcs::ResolveProjectileWallCollisions(m_registry);
    if (GameEcs::HasDefeatedPlayer(m_registry))
    {
        GameEcs::DrawDefeatOverlay(frame);
        if (GameEcs::IsAnyReturnInputPressed())
        {
            MainMenu();
        }
        return;
    }

    GameEcs::UpdateEnemies(m_registry, frame);
    GameEcs::UpdateIFrames(m_registry, frame);
    GameEcs::ResolveEnemyPlayerCollisions(m_registry, m_lua);

    GameEcs::DrawEnemies(m_registry);
    GameEcs::DrawProjectiles(m_registry);
    GameEcs::DrawWalls(m_registry);
    GameEcs::DrawPlayersAndHud(m_registry);
}

void EngineAPI::SetSpellConfig(int projectileCount, float projectileSpeed, float projectileSize, bool ricochet, int damage, float colorR, float colorG, float colorB, float staggerDelay)
{
    GameEcs::SpellTuning tuning{};
    tuning.projectileCount = projectileCount;
    tuning.projectileSpeed = projectileSpeed;
    tuning.projectileSize = projectileSize;
    tuning.ricochet = ricochet;
    tuning.damage = damage;
    tuning.staggerDelay = std::max(0.0f, staggerDelay);
    tuning.projectileColor = Color{
        static_cast<unsigned char>(colorR * 255.0f),
        static_cast<unsigned char>(colorG * 255.0f),
        static_cast<unsigned char>(colorB * 255.0f),
        255};
    GameEcs::SetSpellTuning(m_registry, tuning);
}

int EngineAPI::NumberKeyPressed1To5() const
{
    if (IsKeyPressed(KEY_ONE))
    {
        return 1;
    }
    if (IsKeyPressed(KEY_TWO))
    {
        return 2;
    }
    if (IsKeyPressed(KEY_THREE))
    {
        return 3;
    }
    if (IsKeyPressed(KEY_FOUR))
    {
        return 4;
    }
    if (IsKeyPressed(KEY_FIVE))
    {
        return 5;
    }

    return 0;
}

bool EngineAPI::NextSpellPressed() const
{
    for (int gamepad = 0; gamepad < 4; ++gamepad)
    {
        if (!IsGamepadAvailable(gamepad))
        {
            continue;
        }

        if (IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_RIGHT_TRIGGER_1))
        {
            return true;
        }
    }

    return false;
}

bool EngineAPI::PrevSpellPressed() const
{
    for (int gamepad = 0; gamepad < 4; ++gamepad)
    {
        if (!IsGamepadAvailable(gamepad))
        {
            continue;
        }

        if (IsGamepadButtonPressed(gamepad, GAMEPAD_BUTTON_LEFT_TRIGGER_1))
        {
            return true;
        }
    }

    return false;
}

float EngineAPI::ScreenWidth() const
{
    return static_cast<float>(GetScreenWidth());
}

float EngineAPI::ScreenHeight() const
{
    return static_cast<float>(GetScreenHeight());
}

float EngineAPI::DeltaTime() const
{
    return GetFrameTime();
}

bool EngineAPI::IsFullscreen() const
{
    return IsWindowState(FLAG_FULLSCREEN_MODE);
}

void EngineAPI::SetFullscreen(bool enable)
{
    if (enable != IsFullscreen())
    {
        ToggleFullscreen();
    }
}

void EngineAPI::SetResolution(int width, int height)
{
    // Only allow resolution changes when not in fullscreen
    if (IsFullscreen())
    {
        return;
    }

    // Clamp to reasonable monitor bounds
    int monitorWidth = GetMonitorWidth(0);
    int monitorHeight = GetMonitorHeight(0);
    width = std::min(width, monitorWidth);
    height = std::min(height, monitorHeight);
    width = std::max(width, 800);
    height = std::max(height, 600);

    // SetWindowMinSize to ensure window is resizable
    SetWindowMinSize(800, 600);

    // For Raylib, we need to set the window state as resizable first
    if (!IsWindowState(FLAG_WINDOW_RESIZABLE))
    {
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    }

    // Raylib doesn't provide direct window resize after creation,
    // so we use a workaround: we'll store the desired size and rely on
    // the window being resizable for user-initiated changes.
    // For programmatic changes, we close and reinit the window.
    CloseWindow();
    InitWindow(width, height, "Arcane Onslaught");
    SetTargetFPS(60);
}

void EngineAPI::SpawnEnemy(float x, float y, float vx, float vy, float radius)
{
    const entt::entity enemy = m_registry.create();
    m_registry.emplace<GameEcs::EnemyTag>(enemy);
    m_registry.emplace<GameEcs::Position>(enemy, Vector2{x, y});
    m_registry.emplace<GameEcs::Velocity>(enemy, Vector2{vx, vy});
    m_registry.emplace<GameEcs::CircleCollider>(enemy, radius);
}

void EngineAPI::SpawnEntity(lua_State *L)
{
    if (L == nullptr || !lua_istable(L, -1))
    {
        lua_pushinteger(L, 0);
        return;
    }

    const uint32_t entityId = static_cast<uint32_t>(GameEcs::ImportEntityFromTable(m_registry, L, -1));
    lua_pushinteger(L, entityId);
}

int EngineAPI::CountEnemies() const
{
    auto enemies = m_registry.view<GameEcs::EnemyTag>();
    return static_cast<int>(enemies.size());
}

entt::registry &EngineAPI::Registry()
{
    return m_registry;
}

const entt::registry &EngineAPI::Registry() const
{
    return m_registry;
}

void EngineAPI::GetAllEntities(lua_State *L)
{
    lua_newtable(L);
    int index = 1;

    for (auto entity : m_registry.storage<entt::entity>())
    {
        lua_pushinteger(L, static_cast<lua_Integer>(static_cast<uint32_t>(entity)));
        lua_seti(L, -2, index);
        ++index;
    }
}

void EngineAPI::GetEntityData(lua_State *L, uint32_t entityId)
{
    entt::entity entity = static_cast<entt::entity>(entityId);

    if (!m_registry.valid(entity))
    {
        lua_pushnil(L);
        return;
    }

    lua_newtable(L);
    const int tableIndex = lua_gettop(L);

    // Position
    if (m_registry.all_of<GameEcs::Position>(entity))
    {
        const auto &pos = m_registry.get<GameEcs::Position>(entity);
        lua_newtable(L);
        lua_pushnumber(L, pos.value.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, pos.value.y);
        lua_setfield(L, -2, "y");
        lua_setfield(L, tableIndex, "position");
    }

    // Velocity
    if (m_registry.all_of<GameEcs::Velocity>(entity))
    {
        const auto &vel = m_registry.get<GameEcs::Velocity>(entity);
        lua_newtable(L);
        lua_pushnumber(L, vel.value.x);
        lua_setfield(L, -2, "x");
        lua_pushnumber(L, vel.value.y);
        lua_setfield(L, -2, "y");
        lua_setfield(L, tableIndex, "velocity");
    }

    // CircleCollider
    if (m_registry.all_of<GameEcs::CircleCollider>(entity))
    {
        const auto &collider = m_registry.get<GameEcs::CircleCollider>(entity);
        lua_newtable(L);
        lua_pushnumber(L, collider.radius);
        lua_setfield(L, -2, "radius");
        lua_setfield(L, tableIndex, "circle_collider");
    }

    // Health
    if (m_registry.all_of<GameEcs::Health>(entity))
    {
        const auto &health = m_registry.get<GameEcs::Health>(entity);
        lua_newtable(L);
        lua_pushinteger(L, health.hp);
        lua_setfield(L, -2, "hp");
        lua_setfield(L, tableIndex, "health");
    }

    // DamageCooldown
    if (m_registry.all_of<GameEcs::DamageCooldown>(entity))
    {
        const auto &cooldown = m_registry.get<GameEcs::DamageCooldown>(entity);
        lua_newtable(L);
        lua_pushnumber(L, cooldown.seconds);
        lua_setfield(L, -2, "seconds");
        lua_setfield(L, tableIndex, "damage_cooldown");
    }

    // Tags
    if (m_registry.all_of<GameEcs::SpellTag>(entity))
    {
        lua_pushboolean(L, 1);
        lua_setfield(L, tableIndex, "spell_tag");
    }
    if (m_registry.all_of<GameEcs::PlayerTag>(entity))
    {
        lua_pushboolean(L, 1);
        lua_setfield(L, tableIndex, "player_tag");
    }
    if (m_registry.all_of<GameEcs::EnemyTag>(entity))
    {
        lua_pushboolean(L, 1);
        lua_setfield(L, tableIndex, "enemy_tag");
    }
    if (m_registry.all_of<GameEcs::ProjectileTag>(entity))
    {
        lua_pushboolean(L, 1);
        lua_setfield(L, tableIndex, "projectile_tag");
    }

    // ProjectileDamage
    if (m_registry.all_of<GameEcs::ProjectileDamage>(entity))
    {
        const auto &damage = m_registry.get<GameEcs::ProjectileDamage>(entity);
        lua_newtable(L);
        lua_pushinteger(L, damage.value);
        lua_setfield(L, -2, "value");
        lua_setfield(L, tableIndex, "projectile_damage");
    }

    // ProjectileRicochet
    if (m_registry.all_of<GameEcs::ProjectileRicochet>(entity))
    {
        const auto &ricochet = m_registry.get<GameEcs::ProjectileRicochet>(entity);
        lua_newtable(L);
        lua_pushinteger(L, ricochet.bouncesLeft);
        lua_setfield(L, -2, "bounces_left");
        lua_setfield(L, tableIndex, "projectile_ricochet");
    }

    // ProjectileColor
    if (m_registry.all_of<GameEcs::ProjectileColor>(entity))
    {
        const auto &color = m_registry.get<GameEcs::ProjectileColor>(entity).value;
        lua_newtable(L);
        lua_pushnumber(L, static_cast<double>(color.r) / 255.0);
        lua_setfield(L, -2, "r");
        lua_pushnumber(L, static_cast<double>(color.g) / 255.0);
        lua_setfield(L, -2, "g");
        lua_pushnumber(L, static_cast<double>(color.b) / 255.0);
        lua_setfield(L, -2, "b");
        lua_setfield(L, tableIndex, "projectile_color");
    }
}

bool EngineAPI::UpdateEntityData(lua_State *L, uint32_t entityId)
{
    entt::entity entity = static_cast<entt::entity>(entityId);

    if (!m_registry.valid(entity) || !lua_istable(L, -1))
    {
        return false;
    }

    const int tableIndex = lua_absindex(L, -1);

    // Helper lambda to read Vector2
    auto readVec2 = [L](int fieldIndex) -> std::pair<Vector2, bool>
    {
        if (!lua_istable(L, fieldIndex))
            return {Vector2{}, false};
        float x = 0, y = 0;
        lua_getfield(L, fieldIndex, "x");
        if (lua_isnumber(L, -1))
            x = static_cast<float>(lua_tonumber(L, -1));
        lua_pop(L, 1);
        lua_getfield(L, fieldIndex, "y");
        if (lua_isnumber(L, -1))
            y = static_cast<float>(lua_tonumber(L, -1));
        lua_pop(L, 1);
        return {Vector2{x, y}, true};
    };

    // Position
    if (lua_getfield(L, tableIndex, "position") != LUA_TNIL)
    {
        auto [vec, ok] = readVec2(lua_gettop(L));
        if (ok)
        {
            if (m_registry.all_of<GameEcs::Position>(entity))
                m_registry.patch<GameEcs::Position>(entity, [vec](auto &pos)
                                                    { pos.value = vec; });
            else
                m_registry.emplace<GameEcs::Position>(entity, vec);
        }
    }
    lua_pop(L, 1);

    // Velocity
    if (lua_getfield(L, tableIndex, "velocity") != LUA_TNIL)
    {
        auto [vec, ok] = readVec2(lua_gettop(L));
        if (ok)
        {
            if (m_registry.all_of<GameEcs::Velocity>(entity))
                m_registry.patch<GameEcs::Velocity>(entity, [vec](auto &vel)
                                                    { vel.value = vec; });
            else
                m_registry.emplace<GameEcs::Velocity>(entity, vec);
        }
    }
    lua_pop(L, 1);

    // CircleCollider
    if (lua_getfield(L, tableIndex, "circle_collider") != LUA_TNIL)
    {
        if (lua_istable(L, -1))
        {
            lua_getfield(L, -1, "radius");
            float radius = static_cast<float>(lua_tonumber(L, -1));
            lua_pop(L, 1);
            if (m_registry.all_of<GameEcs::CircleCollider>(entity))
                m_registry.patch<GameEcs::CircleCollider>(entity, [radius](auto &col)
                                                          { col.radius = radius; });
            else
                m_registry.emplace<GameEcs::CircleCollider>(entity, radius);
        }
    }
    lua_pop(L, 1);

    // Health
    if (lua_getfield(L, tableIndex, "health") != LUA_TNIL)
    {
        if (lua_istable(L, -1))
        {
            lua_getfield(L, -1, "hp");
            int hp = static_cast<int>(lua_tointeger(L, -1));
            lua_pop(L, 1);
            if (m_registry.all_of<GameEcs::Health>(entity))
                m_registry.patch<GameEcs::Health>(entity, [hp](auto &h)
                                                  { h.hp = hp; });
            else
                m_registry.emplace<GameEcs::Health>(entity, hp);
        }
    }
    lua_pop(L, 1);

    // DamageCooldown
    if (lua_getfield(L, tableIndex, "damage_cooldown") != LUA_TNIL)
    {
        if (lua_istable(L, -1))
        {
            lua_getfield(L, -1, "seconds");
            float seconds = static_cast<float>(lua_tonumber(L, -1));
            lua_pop(L, 1);
            if (m_registry.all_of<GameEcs::DamageCooldown>(entity))
                m_registry.patch<GameEcs::DamageCooldown>(entity, [seconds](auto &cd)
                                                          { cd.seconds = seconds; });
            else
                m_registry.emplace<GameEcs::DamageCooldown>(entity, seconds);
        }
    }
    lua_pop(L, 1);

    // Tags (these are added/removed as needed)
    bool hasSpellTag = false;
    lua_getfield(L, tableIndex, "spell_tag");
    if (lua_toboolean(L, -1))
        hasSpellTag = true;
    lua_pop(L, 1);

    if (hasSpellTag && !m_registry.all_of<GameEcs::SpellTag>(entity))
        m_registry.emplace<GameEcs::SpellTag>(entity);
    else if (!hasSpellTag && m_registry.all_of<GameEcs::SpellTag>(entity))
        m_registry.remove<GameEcs::SpellTag>(entity);

    bool hasPlayerTag = false;
    lua_getfield(L, tableIndex, "player_tag");
    if (lua_toboolean(L, -1))
        hasPlayerTag = true;
    lua_pop(L, 1);

    if (hasPlayerTag && !m_registry.all_of<GameEcs::PlayerTag>(entity))
        m_registry.emplace<GameEcs::PlayerTag>(entity);
    else if (!hasPlayerTag && m_registry.all_of<GameEcs::PlayerTag>(entity))
        m_registry.remove<GameEcs::PlayerTag>(entity);

    bool hasEnemyTag = false;
    lua_getfield(L, tableIndex, "enemy_tag");
    if (lua_toboolean(L, -1))
        hasEnemyTag = true;
    lua_pop(L, 1);

    if (hasEnemyTag && !m_registry.all_of<GameEcs::EnemyTag>(entity))
        m_registry.emplace<GameEcs::EnemyTag>(entity);
    else if (!hasEnemyTag && m_registry.all_of<GameEcs::EnemyTag>(entity))
        m_registry.remove<GameEcs::EnemyTag>(entity);

    bool hasProjectileTag = false;
    lua_getfield(L, tableIndex, "projectile_tag");
    if (lua_toboolean(L, -1))
        hasProjectileTag = true;
    lua_pop(L, 1);

    if (hasProjectileTag && !m_registry.all_of<GameEcs::ProjectileTag>(entity))
        m_registry.emplace<GameEcs::ProjectileTag>(entity);
    else if (!hasProjectileTag && m_registry.all_of<GameEcs::ProjectileTag>(entity))
        m_registry.remove<GameEcs::ProjectileTag>(entity);

    // ProjectileDamage
    if (lua_getfield(L, tableIndex, "projectile_damage") != LUA_TNIL)
    {
        if (lua_istable(L, -1))
        {
            lua_getfield(L, -1, "value");
            int damage = static_cast<int>(lua_tointeger(L, -1));
            lua_pop(L, 1);
            if (m_registry.all_of<GameEcs::ProjectileDamage>(entity))
                m_registry.patch<GameEcs::ProjectileDamage>(entity, [damage](auto &pd)
                                                            { pd.value = damage; });
            else
                m_registry.emplace<GameEcs::ProjectileDamage>(entity, damage);
        }
    }
    lua_pop(L, 1);

    // ProjectileRicochet
    if (lua_getfield(L, tableIndex, "projectile_ricochet") != LUA_TNIL)
    {
        if (lua_istable(L, -1))
        {
            lua_getfield(L, -1, "bounces_left");
            int bounces = static_cast<int>(lua_tointeger(L, -1));
            lua_pop(L, 1);
            if (m_registry.all_of<GameEcs::ProjectileRicochet>(entity))
                m_registry.patch<GameEcs::ProjectileRicochet>(entity, [bounces](auto &pr)
                                                              { pr.bouncesLeft = bounces; });
            else
                m_registry.emplace<GameEcs::ProjectileRicochet>(entity, bounces);
        }
    }
    lua_pop(L, 1);

    // ProjectileColor
    if (lua_getfield(L, tableIndex, "projectile_color") != LUA_TNIL)
    {
        if (lua_istable(L, -1))
        {
            float r = 1.0f, g = 0.82f, b = 0.33f;
            lua_getfield(L, -1, "r");
            if (lua_isnumber(L, -1))
                r = static_cast<float>(lua_tonumber(L, -1));
            lua_pop(L, 1);
            lua_getfield(L, -1, "g");
            if (lua_isnumber(L, -1))
                g = static_cast<float>(lua_tonumber(L, -1));
            lua_pop(L, 1);
            lua_getfield(L, -1, "b");
            if (lua_isnumber(L, -1))
                b = static_cast<float>(lua_tonumber(L, -1));
            lua_pop(L, 1);

            Color color{
                static_cast<unsigned char>(std::clamp(r, 0.0f, 1.0f) * 255.0f),
                static_cast<unsigned char>(std::clamp(g, 0.0f, 1.0f) * 255.0f),
                static_cast<unsigned char>(std::clamp(b, 0.0f, 1.0f) * 255.0f),
                255};
            if (m_registry.all_of<GameEcs::ProjectileColor>(entity))
                m_registry.patch<GameEcs::ProjectileColor>(entity, [color](auto &pc)
                                                           { pc.value = color; });
            else
                m_registry.emplace<GameEcs::ProjectileColor>(entity, color);
        }
    }
    lua_pop(L, 1);

    return true;
}

void EngineAPI::DeleteEntity(uint32_t entityId)
{
    entt::entity entity = static_cast<entt::entity>(entityId);
    if (m_registry.valid(entity))
    {
        m_registry.destroy(entity);
    }
}

void EngineAPI::AttachBehavior(uint32_t entityId, lua_State *L)
{
    if (L == nullptr || !lua_isthread(L, -1))
    {
        lua_pop(L, 1);
        return;
    }

    entt::entity entity = static_cast<entt::entity>(entityId);
    if (!m_registry.valid(entity))
    {
        lua_pop(L, 1);
        return;
    }

    // Create a reference to the co-routine
    lua_pushvalue(L, -1);
    int coroutineRef = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pop(L, 1);

    // Add or update the Behavior component
    if (m_registry.all_of<GameEcs::Behavior>(entity))
    {
        auto &behavior = m_registry.get<GameEcs::Behavior>(entity);
        if (behavior.coroutineRef != LUA_NOREF)
        {
            luaL_unref(L, LUA_REGISTRYINDEX, behavior.coroutineRef);
        }
        behavior.coroutineRef = coroutineRef;
    }
    else
    {
        m_registry.emplace<GameEcs::Behavior>(entity, coroutineRef);
    }
}
