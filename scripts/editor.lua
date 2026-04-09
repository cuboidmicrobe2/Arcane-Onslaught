local intro_timer = 0.0
dofile("scripts/ui_common.lua")

local function return_button()
	local layout = UI.get_default_menu_layout()

	local start_y = screen_height() * 0.64

	UI.render_menu_button("return", "Return", start_y, 2, layout, main_menu)
end

function update_editor()
	intro_timer = intro_timer + delta_time()
	return_button()
end