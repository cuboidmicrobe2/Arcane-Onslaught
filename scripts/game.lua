dofile("scripts/ui_common.lua")
dofile("scripts/spell_config.lua")
local GameScene = dofile("scripts/game_scene.lua")

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

    engine.run_game()

    local layout = UI.get_default_menu_layout()
    layout.width = 180
    layout.button_height = 44

    if engine.button("return", 24, engine.screen_height() - 68, layout.width, layout.button_height, "Return") then
        engine.main_menu()
    end

    engine.text(spell.name, 24, 0, engine.screen_height() - 34, engine.screen_width(), 24)
end

return GameScene