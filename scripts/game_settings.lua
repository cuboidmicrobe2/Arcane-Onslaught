-- Game settings (not window settings)
-- Stored in scripts/game_settings_save.lua

local GameSettings = {}

local settings = {
    endless_mode = false
}

local SETTINGS_PATH = "scripts/game_settings_save.lua"

function GameSettings.load()
    local chunk = loadfile(SETTINGS_PATH)
    if not chunk then
        -- File doesn't exist, use defaults
        return
    end

    local ok, data = pcall(chunk)
    if ok and type(data) == "table" then
        if data.endless_mode ~= nil then
            settings.endless_mode = data.endless_mode
        end
    end
end

function GameSettings.save()
    local file = io.open(SETTINGS_PATH, "w")
    if not file then
        return false
    end

    file:write("return {\n")
    file:write("    endless_mode = ", tostring(settings.endless_mode), ",\n")
    file:write("}\n")
    file:close()
    return true
end

function GameSettings.get_endless_mode()
    return settings.endless_mode
end

function GameSettings.set_endless_mode(enabled)
    settings.endless_mode = enabled
    GameSettings.save()
end

-- Load settings on script load
GameSettings.load()

return GameSettings
