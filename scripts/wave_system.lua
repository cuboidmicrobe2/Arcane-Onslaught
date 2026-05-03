local ECS = dofile("scripts/ecs.lua")

local WaveSystem = {}

-- Wave configuration
local WAVES = {
    { enemy_count = 3, spawn_interval = 1.0 },
    { enemy_count = 5, spawn_interval = 0.8 },
    { enemy_count = 7, spawn_interval = 0.7 },
    { enemy_count = 10, spawn_interval = 0.6 },
    { enemy_count = 12, spawn_interval = 0.5 },
}

local wave_state = {
    current_wave = 1,
    wave_active = false,
    enemies_spawned = 0,
    spawn_timer = 0.0,
    paused = false,
    game_won = false,
    player_pos = ECS.vec2(0, 0)  -- Will be set in init
}

-- Enemy spawn patterns - spawn at edges and move towards player
local function get_enemy_spawn_pattern(index, wave_num, player_pos)
    local screen_width = engine.screen_width()
    local screen_height = engine.screen_height()
    local pattern = index % 4
    local base_speed = 150.0 + (wave_num * 20.0)  -- Increase speed with waves
    local radius = 14.0 + wave_num
    
    -- Spawn positions at screen edges
    local spawn_positions = {
        { x = screen_width * 0.1, y = 0 },           -- Top left
        { x = screen_width * 0.9, y = 0 },           -- Top right
        { x = 0, y = screen_height * 0.3 },          -- Left side
        { x = screen_width, y = screen_height * 0.3 }, -- Right side
    }
    
    local spawn = spawn_positions[pattern + 1]
    local spawn_pos = ECS.vec2(spawn.x, spawn.y)
    
    -- Calculate direction towards player
    local dx = player_pos.x - spawn.x
    local dy = player_pos.y - spawn.y
    local dist = math.sqrt(dx * dx + dy * dy)
    
    -- Normalize and scale velocity
    local vel_x = 0
    local vel_y = 0
    if dist > 0 then
        vel_x = (dx / dist) * base_speed
        vel_y = (dy / dist) * base_speed
    end
    
    return { 
        pos = spawn_pos, 
        vel = ECS.vec2(vel_x, vel_y), 
        radius = radius 
    }
end

function WaveSystem.init()
    wave_state.current_wave = 1
    wave_state.wave_active = false
    wave_state.enemies_spawned = 0
    wave_state.spawn_timer = 0.0
    wave_state.paused = false
    wave_state.last_enemy_count = 0
    wave_state.game_won = false
    
    -- Set player position (must match game_scene.lua spawn position)
    wave_state.player_pos = ECS.vec2(engine.screen_width() * 0.5, engine.screen_height() * 0.72)
end

function WaveSystem.start_wave()
    if not wave_state.wave_active then
        wave_state.wave_active = true
        wave_state.enemies_spawned = 0
        wave_state.spawn_timer = 0.0
        wave_state.paused = false
    end
end

function WaveSystem.update(delta_time)
    if not wave_state.wave_active or wave_state.paused then
        return
    end
    
    local current_wave = WAVES[wave_state.current_wave]
    if not current_wave then
        -- All waves defeated - loop back or handle game over
        return
    end
    
    -- Spawn enemies at intervals
    wave_state.spawn_timer = wave_state.spawn_timer + delta_time
    
    if wave_state.spawn_timer >= current_wave.spawn_interval and 
       wave_state.enemies_spawned < current_wave.enemy_count then
        
        local enemy_data = get_enemy_spawn_pattern(wave_state.enemies_spawned, wave_state.current_wave, wave_state.player_pos)
        
        -- Create enemy entity from Lua table and spawn it via engine
        local enemy_entity = ECS.enemy(enemy_data.pos, enemy_data.vel, enemy_data.radius)
        
        -- Scale health by wave number: 1.1, 1.2, 1.3, etc.
        local health_multiplier = 1.0 + (wave_state.current_wave * 0.1)
        enemy_entity.health.hp = math.max(1, math.floor(enemy_entity.health.hp * health_multiplier + 0.5))
        
        engine.spawn_entity(enemy_entity)
        
        wave_state.enemies_spawned = wave_state.enemies_spawned + 1
        wave_state.spawn_timer = 0.0
    end
    
    -- Check if all enemies for this wave are spawned and defeated
    if wave_state.enemies_spawned >= current_wave.enemy_count then
        -- Only check enemy count after all enemies are spawned
        -- Cache the count to avoid multiple registry accesses in the same frame
        wave_state.last_enemy_count = engine.count_enemies()
        
        if wave_state.last_enemy_count == 0 then
            WaveSystem.next_wave()
        end
    else
        -- While still spawning, update the enemy count
        wave_state.last_enemy_count = engine.count_enemies()
    end
end

function WaveSystem.next_wave()
    if wave_state.current_wave < #WAVES then
        wave_state.current_wave = wave_state.current_wave + 1
        wave_state.enemies_spawned = 0
        wave_state.spawn_timer = 0.0
        wave_state.wave_active = true
    else
        -- All waves completed
        wave_state.wave_active = false
        wave_state.game_won = true
    end
end

function WaveSystem.get_current_wave()
    return wave_state.current_wave
end

function WaveSystem.get_wave_active()
    return wave_state.wave_active
end

function WaveSystem.is_game_won()
    return wave_state.game_won
end

function WaveSystem.pause_wave()
    wave_state.paused = true
end

function WaveSystem.resume_wave()
    wave_state.paused = false
end

function WaveSystem.get_state()
    local current_wave = WAVES[wave_state.current_wave]
    local total_enemies = current_wave and current_wave.enemy_count or 0
    
    return {
        current_wave = wave_state.current_wave,
        wave_active = wave_state.wave_active,
        enemies_spawned = wave_state.enemies_spawned,
        total_enemies_in_wave = total_enemies,
        enemies_remaining = wave_state.last_enemy_count or 0,
        paused = wave_state.paused
    }
end

return WaveSystem
