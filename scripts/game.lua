dofile("scripts/ui_common.lua")
dofile("scripts/spell_config.lua")
local GameScene = dofile("scripts/game_scene.lua")

local WaveSystem = GameScene.wave_system
local wave_started = false

function update_game()
    if SpellConfig.handle_slot_input() then
        SpellConfig.save_to_file()
    end

    local spell = SpellConfig.get_active_spell()
    engine.set_spell_config(
        spell.projectile_count,
        spell.projectile_speed,
        spell.projectile_size,
        spell.ricochet,
        spell.damage
    )

    -- Start first wave on first frame
    if not wave_started then
        WaveSystem.start_wave()
        wave_started = true
    end

    -- Update wave system
    WaveSystem.update(engine.delta_time())

    engine.run_game()

    local layout = UI.get_default_menu_layout()
    layout.width = 180
    layout.button_height = 44

    if engine.button("return", 24, engine.screen_height() - 68, layout.width, layout.button_height, "Return") then
        engine.main_menu()
    end

    -- Draw wave information on the right
    local wave_state = WaveSystem.get_state()
    local wave_text = string.format("Wave: %d | Spawned: %d/%d | Alive: %d", 
        wave_state.current_wave, 
        wave_state.enemies_spawned,
        wave_state.total_enemies_in_wave,
        wave_state.enemies_remaining)
    
    engine.text(wave_text, 24, engine.screen_width() - 524, 24, 500, 24)
    engine.text(spell.name, 24, 0, engine.screen_height() - 34, engine.screen_width(), 24)
    
    -- Display you win text after defeating all waves
    if WaveSystem.is_game_won() then
        local screen_width = engine.screen_width()
        local screen_height = engine.screen_height()
        engine.text("YOU WIN!", 72, 0, screen_height / 2 - 36, screen_width, 72)
    end
end

return GameScene