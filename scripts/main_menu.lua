local intro_timer = 0.0
dofile("scripts/ui_common.lua")

function update_menu()
	intro_timer = intro_timer + engine.delta_time()

	local layout = UI.get_default_menu_layout()

	local start_y = engine.screen_height() * 0.42

	engine.text("Arcane Onslaught", 72, 0, engine.screen_height() * 0.20, engine.screen_width(), 60)

	UI.render_menu_button("play", "Play", start_y, 0, layout, engine.start_game)
	UI.render_menu_button("editor", "Spell Editor", start_y, 1, layout, engine.open_editor)
	UI.render_menu_button("settings", "Settings", start_y, 2, layout, engine.open_settings)
	UI.render_menu_button("quit", "Quit", start_y, 3, layout, engine.quit_game)
end