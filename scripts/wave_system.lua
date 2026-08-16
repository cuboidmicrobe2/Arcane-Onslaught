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
    player_pos = ECS.vec2(0, 0),
    endless_mode = false,
    wave_sequence = nil,
    should_advance_wave = false  -- Flag to advance on next update
}

local function get_wave_config()
    local current_wave = WAVES[wave_state.current_wave]
    if current_wave then
        return current_wave
    end

    if wave_state.endless_mode then
        local wave_num = wave_state.current_wave
        return {
            enemy_count = 3 + (wave_num - 1) * 2,
            spawn_interval = math.max(0.2, 0.6 - (wave_num - 5) * 0.05)
        }
    end

    return nil
end

-- Enemy spawn patterns - spawn at edges and move towards player
local function get_enemy_spawn_pattern(index, wave_num, player_pos)
    local screen_width = engine.screen_width()
    local screen_height = engine.screen_height()
    local pattern = index % 4
    local base_speed = 150.0 + (wave_num * 20.0)
    local radius = 14.0 + wave_num

    local spawn_positions = {
        { x = screen_width * 0.1, y = 0 },
        { x = screen_width * 0.9, y = 0 },
        { x = 0, y = screen_height * 0.3 },
        { x = screen_width, y = screen_height * 0.3 },
    }

    local spawn = spawn_positions[pattern + 1]
    local spawn_pos = ECS.vec2(spawn.x, spawn.y)

    local dx = player_pos.x - spawn.x
    local dy = player_pos.y - spawn.y
    local dist = math.sqrt(dx * dx + dy * dy)

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

local function spawn_enemy_for_index(index)
    local enemy_data = get_enemy_spawn_pattern(index, wave_state.current_wave, wave_state.player_pos)
    local enemy_entity = ECS.enemy(enemy_data.pos, ECS.vec2(0, 0), enemy_data.radius)

    local health_multiplier = 1.0 + (wave_state.current_wave * 0.1)
    enemy_entity.health.hp = math.max(1, math.floor(enemy_entity.health.hp * health_multiplier + 0.5))

    local entity_id = engine.spawn_entity(enemy_entity)
    engine.attach_behavior(entity_id, ECS.enemy_entrance(entity_id, enemy_data.vel, 10.0))
    wave_state.enemies_spawned = wave_state.enemies_spawned + 1
    wave_state.last_enemy_count = engine.count_enemies()
end

local function create_wave_sequence()
    return coroutine.create(function()
        local config = get_wave_config()
        if not config then
            wave_state.wave_active = false
            wave_state.game_won = true
            wave_state.wave_sequence = nil
            return
        end

        for i = 1, config.enemy_count do
            local elapsed = 0.0
            local delay = config.spawn_interval
            while elapsed < delay do
                elapsed = elapsed + engine.delta_time()
                engine.coroutine_yield_frame()
            end

            spawn_enemy_for_index(i - 1)
        end

        while engine.count_enemies() > 0 do
            wave_state.last_enemy_count = engine.count_enemies()
            engine.coroutine_yield_frame()
        end

        wave_state.last_enemy_count = 0
        wave_state.wave_sequence = nil

        -- Set flag to advance wave on next update (don't call next_wave from within coroutine)
        if wave_state.endless_mode or wave_state.current_wave < #WAVES then
            wave_state.should_advance_wave = true
        else
            wave_state.wave_active = false
            wave_state.game_won = true
        end
    end)
end

function WaveSystem.init()
    wave_state.current_wave = 1
    wave_state.wave_active = false
    wave_state.enemies_spawned = 0
    wave_state.spawn_timer = 0.0
    wave_state.paused = false
    wave_state.last_enemy_count = 0
    wave_state.game_won = false
    wave_state.wave_sequence = nil
    wave_state.should_advance_wave = false

    wave_state.player_pos = ECS.vec2(engine.screen_width() * 0.5, engine.screen_height() * 0.72)
end

function WaveSystem.start_wave()
    if wave_state.wave_sequence ~= nil or wave_state.game_won then
        return
    end

    wave_state.wave_active = true
    wave_state.enemies_spawned = 0
    wave_state.spawn_timer = 0.0
    wave_state.paused = false
    wave_state.wave_sequence = create_wave_sequence()
end

function WaveSystem.update(delta_time)
    if wave_state.paused then
        return
    end

    -- Check if we should advance to next wave (set by coroutine when it completes)
    if wave_state.should_advance_wave then
        wave_state.should_advance_wave = false
        WaveSystem.next_wave()
    end

    if wave_state.wave_active and wave_state.wave_sequence == nil then
        WaveSystem.start_wave()
    end

    if not wave_state.wave_active or wave_state.wave_sequence == nil then
        return
    end

    local ok, err = coroutine.resume(wave_state.wave_sequence)
    if not ok then
        print("Wave coroutine error: " .. tostring(err))
        wave_state.wave_sequence = nil
        wave_state.wave_active = false
        return
    end

    if wave_state.wave_sequence ~= nil and coroutine.status(wave_state.wave_sequence) == "dead" then
        wave_state.wave_sequence = nil
    end

    wave_state.last_enemy_count = engine.count_enemies()
end

function WaveSystem.next_wave()
    if wave_state.current_wave < #WAVES then
        wave_state.current_wave = wave_state.current_wave + 1
    elseif wave_state.endless_mode then
        wave_state.current_wave = wave_state.current_wave + 1
    else
        wave_state.wave_active = false
        wave_state.game_won = true
        return
    end

    wave_state.enemies_spawned = 0
    wave_state.spawn_timer = 0.0
    wave_state.wave_active = true
    wave_state.wave_sequence = nil
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
    local current_wave = get_wave_config()
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

function WaveSystem.set_endless_mode(enabled)
    wave_state.endless_mode = enabled
end

function WaveSystem.is_endless_mode()
    return wave_state.endless_mode
end

return WaveSystem
