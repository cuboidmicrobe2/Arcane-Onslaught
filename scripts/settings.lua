dofile("scripts/ui_common.lua")
local GameSettings = dofile("scripts/game_settings.lua")

local Settings = {}

-- Available resolutions
local RESOLUTIONS = {
    { width = 1280, height = 720, label = "1280x720" },
    { width = 1920, height = 1080, label = "1920x1080" },
    { width = 2560, height = 1440, label = "2560x1440" },
}

local selected_resolution_index = 1
local fullscreen_enabled = false
local endless_mode_enabled = false

function Settings.initialize()
    fullscreen_enabled = engine.is_fullscreen()
    endless_mode_enabled = GameSettings.get_endless_mode()
    
    -- Find the closest resolution to current screen size
    local current_width = engine.screen_width()
    local current_height = engine.screen_height()
    
    for i, res in ipairs(RESOLUTIONS) do
        if res.width == current_width and res.height == current_height then
            selected_resolution_index = i
            break
        end
    end
end

function update_settings()
    local layout = UI.get_default_menu_layout()
    local start_y = engine.screen_height() * 0.20

    -- Title
    engine.text("Settings", 64, 0, engine.screen_height() * 0.05, engine.screen_width(), 50)

    -- Fullscreen toggle
    local old_fullscreen = fullscreen_enabled
    fullscreen_enabled = engine.checkbox("fullscreen_toggle", fullscreen_enabled, 
        UI.get_centered_x(20), start_y, 20, "Fullscreen")
    
    -- Apply fullscreen change immediately
    if fullscreen_enabled ~= old_fullscreen then
        engine.set_fullscreen(fullscreen_enabled)
    end

    -- Endless mode toggle
    local old_endless = endless_mode_enabled
    endless_mode_enabled = engine.checkbox("endless_mode_toggle", endless_mode_enabled,
        UI.get_centered_x(20), start_y + 50, 20, "Endless Mode")
    
    -- Save endless mode preference
    if endless_mode_enabled ~= old_endless then
        GameSettings.set_endless_mode(endless_mode_enabled)
    end

    -- Resolution selection
    engine.text("Resolution:", 24, 
        UI.get_centered_x(layout.width), start_y + 110, layout.width, 30)

    for i, res in ipairs(RESOLUTIONS) do
        local button_y = start_y + 150 + (i - 1) * (layout.button_height + layout.spacing)
        local button_id = "res_" .. i
        
        -- Only apply resolution if not in fullscreen
        if engine.button(button_id, UI.get_centered_x(layout.width), button_y, layout.width, layout.button_height, res.label) then
            if not fullscreen_enabled then
                selected_resolution_index = i
                engine.set_resolution(res.width, res.height)
            end
        end
        
        -- Draw disabled overlay if in fullscreen
        if fullscreen_enabled then
            engine.draw_rect(
                UI.get_centered_x(layout.width),
                button_y,
                layout.width,
                layout.button_height,
                0, 0, 0, 100
            )
        end
    end

    -- Back button
    local back_y = engine.screen_height() - 100
    if engine.button("back", UI.get_centered_x(layout.width), back_y, layout.width, layout.button_height, "Back") then
        engine.main_menu()
    end
end

Settings.initialize()
