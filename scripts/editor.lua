local intro_timer = 0.0
dofile("scripts/ui_common.lua")
dofile("scripts/spell_config.lua")
local GameScene = dofile("scripts/game_scene.lua")

local editor_root = {
	y = 50
}

local spell_sidebar_root = {
	x = 250,
	y = 240
}

local function editor_y(local_y)
	return local_y + editor_root.y
end

local function spell_sidebar_x(local_x)
	return spell_sidebar_root.x + local_x
end

local function spell_sidebar_y(local_y)
	return editor_y(spell_sidebar_root.y + local_y)
end

local function draw_spell_slot_buttons()
	local x = 0
	local y = 48
	local width = 250
	local height = 46
	local spacing = 12

	engine.text(SpellConfig.slot_title, 26, spell_sidebar_x(0), spell_sidebar_y(0), width, 28)

	for i = 1, SpellConfig.slot_count do
		local spell = SpellConfig.spells[i] or SpellConfig.get_active_spell()
		local prefix = (i == SpellConfig.active_slot) and "> " or ""
		local label = prefix .. tostring(i) .. ". " .. tostring(spell.name)

		if engine.button(
			"spell_slot_" .. tostring(i),
			spell_sidebar_x(x),
			spell_sidebar_y(y + ((i - 1) * (height + spacing))),
			width,
			height,
			label
		) then
			if SpellConfig.active_slot ~= i then
				SpellConfig.set_active_slot(i)
				SpellConfig.save_to_file()
			end
		end
	end
end

local function draw_spell_editor()
	local spell = SpellConfig.get_active_spell()
	local old_name = spell.name
	local old_count = spell.projectile_count
	local old_speed = spell.projectile_speed
	local old_size = spell.projectile_size
	local old_ricochet = spell.ricochet

	local panel_width = 520
	local panel_x = (engine.screen_width() - panel_width) * 0.5
	local slider_width = 360
	local controls_x = (engine.screen_width() - slider_width) * 0.5
	local name_y = 150
	local top_y = 188

	engine.text("Spell Editor", 44, 0, editor_y(25), engine.screen_width(), 50)
	
	engine.text(
				"Active Slot: " .. tostring(SpellConfig.active_slot) .. " (" .. SpellConfig.get_slot_range_hint() .. ", LB/RB)",
				20,
				0,
				editor_y(75),
				engine.screen_width(),
				24
			)

	spell.name = engine.text_field(
		"spell_name_" .. tostring(SpellConfig.active_slot),
		spell.name,
		24,
		controls_x,
		editor_y(name_y),
		slider_width,
		42,
		"Spell Name"
	)

	local row = 1
	local row_step = 82

	spell.projectile_count = math.floor(engine.slider(
		"spell_projectile_count",
		spell.projectile_count,
		1,
		12,
		controls_x,
		editor_y(top_y + (row * row_step)),
		slider_width,
		"Projectile Count"
	) + 0.5)

	row = row + 1
	spell.projectile_speed = math.floor(engine.slider(
		"spell_projectile_speed",
		spell.projectile_speed,
		120,
		900,
		controls_x,
		editor_y(top_y + (row * row_step)),
		slider_width,
		"Projectile Speed"
	) + 0.5)

	row = row + 1
	spell.projectile_size = math.floor(engine.slider(
		"spell_projectile_size",
		spell.projectile_size,
		4,
		36,
		controls_x,
		editor_y(top_y + (row * row_step)),
		slider_width,
		"Projectile Size"
	) + 0.5)

	row = row + 1
	spell.ricochet = engine.checkbox(
		"spell_ricochet",
		spell.ricochet,
		controls_x,
		editor_y(top_y + (row * row_step)),
		28,
		"Ricochet"
	)

	SpellConfig.recalculate_damage(spell)

	if old_name ~= spell.name or
		old_count ~= spell.projectile_count or
		old_speed ~= spell.projectile_speed or
		old_size ~= spell.projectile_size or
		old_ricochet ~= spell.ricochet then
		SpellConfig.save_to_file()
	end

	local ricochet_label = spell.ricochet and "Ricochet: Enabled" or "Ricochet: Disabled"
	local summary_y = top_y + ((row + 1) * row_step) + 10

	engine.text("Calculated Damage: " .. tostring(spell.damage), 30, panel_x, editor_y(summary_y + 34), panel_width, 30)
end

local function return_button()
	local layout = UI.get_default_menu_layout()

	local start_y = engine.screen_height() * 0.80

	UI.render_menu_button("return", "Return", editor_y(start_y), 0, layout, engine.main_menu)
end

local function draw_ecs_summary()
	local entities = (GameScene.ecs and GameScene.ecs.entities) or {}
	local entity_count = #entities
	local summary_y = editor_y(470)

	engine.text("Gameplay ECS", 28, 0, summary_y, engine.screen_width(), 30)
	engine.text("Entities in scene: " .. tostring(entity_count), 20, 0, summary_y + 34, engine.screen_width(), 24)
end

function update_editor()
	intro_timer = intro_timer + engine.delta_time()
	if SpellConfig.handle_slot_input() then
		SpellConfig.save_to_file()
	end
	draw_spell_slot_buttons()
	draw_spell_editor()
	draw_ecs_summary()
	return_button()
end