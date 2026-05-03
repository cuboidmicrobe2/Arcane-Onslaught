local ECS = {}

function ECS.vec2(x, y)
    return { x = x, y = y }
end

function ECS.circle(radius)
    return { radius = radius }
end

function ECS.health(hp)
    return { hp = hp }
end

function ECS.cooldown(seconds)
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
        damage_cooldown = ECS.cooldown(0.0)
    })
end

function ECS.enemy(position, velocity, radius)
    return ECS.entity({
        enemy_tag = true,
        position = position,
        velocity = velocity,
        circle_collider = ECS.circle(radius)
    })
end

function ECS.scene(entities)
    return {
        ecs = {
            entities = entities or {}
        }
    }
end

return ECS
