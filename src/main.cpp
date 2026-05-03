#include <raylib.h>

#include "AppRuntime.hpp"

int main()
{
    InitWindow(2560, 1440, "Arcane Onslaught");
    SetTargetFPS(60);
    SetExitKey(KEY_F1);

    AppRuntime app;
    if (!app.Initialize())
    {
        app.Shutdown();
        CloseWindow();
        return 1;
    }

    app.Run();
    app.Shutdown();
    CloseWindow();
    return 0;
}