local intro_timer = 0.0

local function center_x(width)
	return (screen_width() - width) * 0.5
end

function update_menu()
	intro_timer = intro_timer + delta_time()

	local menu_width = 260
	local button_height = 54
	local spacing = 16

	local start_y = screen_height() * 0.42
	local x = center_x(menu_width)

	text("Arcane Onslaught", 72, 0, screen_height() * 0.20, screen_width(), 60)

	if button("play", x, start_y, menu_width, button_height, "Play") then
		start_game()
	end

	if button("editor", x, start_y + (button_height + spacing), menu_width, button_height, "Spell Editor") then
		open_editor()
	end

	if button("quit", x, start_y + ((button_height + spacing) * 2.0), menu_width, button_height, "Quit") then
		quit_game()
	end
end