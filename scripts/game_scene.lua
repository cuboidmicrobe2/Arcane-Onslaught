local ECS = dofile("scripts/ecs.lua")
local WaveSystem = dofile("scripts/wave_system.lua")

local width = engine.screen_width()
local height = engine.screen_height()

-- Initialize wave system
WaveSystem.init()

local GameScene = ECS.scene({
    ECS.player(ECS.vec2(width * 0.5, height * 0.72))
})

-- Store wave system in scene for access in game loop
GameScene.wave_system = WaveSystem

return GameScene

