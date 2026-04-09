local intro_timer = 0.0
dofile("scripts/ui_common.lua")

function update_menu()
	intro_timer = intro_timer + delta_time()

	local layout = UI.get_default_menu_layout()

	local start_y = screen_height() * 0.42

	text("Arcane Onslaught", 72, 0, screen_height() * 0.20, screen_width(), 60)

	UI.render_menu_button("play", "Play", start_y, 0, layout, start_game)
	UI.render_menu_button("editor", "Spell Editor", start_y, 1, layout, open_editor)
	UI.render_menu_button("quit", "Quit", start_y, 2, layout, quit_game)
end