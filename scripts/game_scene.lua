local ECS = dofile("scripts/ecs.lua")

local width = engine.screen_width()
local height = engine.screen_height()

local GameScene = ECS.scene({
    ECS.player(ECS.vec2(width * 0.5, height * 0.72)),
    ECS.enemy(ECS.vec2(width * 0.5, height * 0.2), ECS.vec2(120.0, 0.0), 16.0),
    ECS.enemy(ECS.vec2(width * 0.25, height * 0.32), ECS.vec2(95.0, 0.0), 14.0),
    ECS.enemy(ECS.vec2(width * 0.75, height * 0.27), ECS.vec2(-110.0, 0.0), 18.0)
})

return GameScene
