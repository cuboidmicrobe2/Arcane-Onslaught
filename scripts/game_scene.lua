local ECS = dofile("scripts/ecs.lua")
local WaveSystem = dofile("scripts/wave_system.lua")

local width = engine.screen_width()
local height = engine.screen_height()

-- Initialize wave system
WaveSystem.init()

local function create_walls()
    local walls = {}
    -- Define wall positions and sizes to create a challenging maze-like layout
    table.insert(walls, ECS.wall(ECS.vec2(width * 0.25, height * 0.25), 150, 40))
    table.insert(walls, ECS.wall(ECS.vec2(width * 0.75, height * 0.25), 150, 40))
    table.insert(walls, ECS.wall(ECS.vec2(width * 0.5, height * 0.4), 40, 120))
    table.insert(walls, ECS.wall(ECS.vec2(width * 0.2, height * 0.55), 120, 40))
    table.insert(walls, ECS.wall(ECS.vec2(width * 0.8, height * 0.55), 120, 40))
    table.insert(walls, ECS.wall(ECS.vec2(width * 0.5, height * 0.7), 140, 35))
    return walls
end

local GameScene = ECS.scene({
    ECS.player(ECS.vec2(width * 0.5, height * 0.72))
})

-- Add walls to the scene
for _, wall in ipairs(create_walls()) do
    table.insert(GameScene.ecs.entities, wall)
end

-- Store wave system in scene for access in game loop
GameScene.wave_system = WaveSystem

return GameScene

