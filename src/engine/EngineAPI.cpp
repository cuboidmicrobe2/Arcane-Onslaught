#include "engine/EngineAPI.hpp"
#include "EngineAPI.hpp"
#include "engine/GameEcs.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>

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

    Rectangle interactionRect{x, y + 8.0f, w, rowHeight};
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

    GameEcs::UpdatePlayers(m_registry, frame);
    GameEcs::SpawnPlayerProjectiles(m_registry, frame);
    GameEcs::UpdateProjectiles(m_registry, frame);
    GameEcs::ResolveProjectileEnemyCollisions(m_registry);
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
    GameEcs::ResolveEnemyPlayerCollisions(m_registry);

    GameEcs::DrawEnemies(m_registry);
    GameEcs::DrawProjectiles(m_registry);
    GameEcs::DrawPlayersAndHud(m_registry);
}

void EngineAPI::SetSpellConfig(int projectileCount, float projectileSpeed, float projectileSize, bool ricochet, int damage)
{
    GameEcs::SpellTuning tuning{};
    tuning.projectileCount = projectileCount;
    tuning.projectileSpeed = projectileSpeed;
    tuning.projectileSize = projectileSize;
    tuning.ricochet = ricochet;
    tuning.damage = damage;
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

entt::registry &EngineAPI::Registry()
{
    return m_registry;
}

const entt::registry &EngineAPI::Registry() const
{
    return m_registry;
}
