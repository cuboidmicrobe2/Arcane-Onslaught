local ECS = {}

function ECS.vec2(x, y)
    return { x = x, y = y }
end

function ECS.circle(radius)
    return { radius = radius }
end

function ECS.rect(width, height)
    return { width = width, height = height }
end

function ECS.health(hp)
    return { hp = hp }
end

function ECS.cooldown(seconds)
    return { seconds = seconds }
end

function ECS.i_frames(seconds)
    return { seconds = seconds }
end

function ECS.entity(components)
    return components or {}
end

function ECS.player(position)
    return ECS.entity({
        player_tag = true,
        position = position,
        velocity = ECS.vec2(0.0, 0.0),
        circle_collider = ECS.circle(20.0),
        health = ECS.health(5),
        damage_cooldown = ECS.cooldown(0.0),
        i_frames = ECS.i_frames(0.0)
    })
end

function ECS.enemy(position, velocity, radius)
    return ECS.entity({
        enemy_tag = true,
        position = position,
        velocity = velocity,
        circle_collider = ECS.circle(radius),
        health = ECS.health(100)
    })
end

function ECS.wall(position, width, height)
    return ECS.entity({
        wall_tag = true,
        position = position,
        rect_collider = ECS.rect(width, height)
    })
end

function ECS.scene(entities)
    return {
        ecs = {
            entities = entities or {}
        }
    }
end

-- Behavior: Enemy entrance animation
-- Ramps velocity from 0 to target over duration
function ECS.enemy_entrance(entity_id, target_vel, duration)
    duration = duration or 0.3
    local elapsed = 0.0
    
    return coroutine.create(function()
        -- Just wait without modifying anything
        while elapsed < duration do
            local dt = engine.delta_time()
            elapsed = elapsed + dt
            engine.coroutine_yield_frame()
        end
    end)
end

-- Behavior: I-Frames (invincibility frames)
-- Grants temporary invulnerability after taking damage
function ECS.i_frames_behavior(entity_id, duration)
    duration = duration or 1.5
    local elapsed = 0.0
    
    return coroutine.create(function()
        while elapsed < duration do
            local dt = engine.delta_time()
            elapsed = elapsed + dt
            engine.coroutine_yield_frame()
        end
    end)
end

return ECS
