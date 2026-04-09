#include "engine/EngineAPI.hpp"

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
