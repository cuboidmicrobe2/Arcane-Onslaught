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

	if SpellConfig.get_spell_count() == 0 then
		if engine.button(
			"add_spell_empty",
			spell_sidebar_x(x),
			spell_sidebar_y(y),
			width,
			height,
			"Add Spell"
		) then
			SpellConfig.add_spell()
			SpellConfig.save_to_file()
		end
		engine.text("No spells created yet.", 22, spell_sidebar_x(0), spell_sidebar_y(y + 70), width, 28)
		return
	end

	for i = 1, SpellConfig.get_spell_count() do
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

	if engine.button(
		"add_spell",
		spell_sidebar_x(x),
		spell_sidebar_y(y + ((SpellConfig.get_spell_count()) * (height + spacing)) + 12),
		width,
		height,
		"Add Spell"
	) then
		SpellConfig.add_spell()
		SpellConfig.save_to_file()
	end
end

local function draw_spell_editor()
	if SpellConfig.get_spell_count() == 0 then
		local panel_width = 520
		local panel_x = (engine.screen_width() - panel_width) * 0.5
		engine.text("Spell Editor", 44, 0, editor_y(25), engine.screen_width(), 50)
		engine.text("Create a spell to begin editing its attributes.", 28, 0, editor_y(120), engine.screen_width(), 32)
		if engine.button(
			"add_spell_editor_button",
			panel_x,
			editor_y(170),
			panel_width,
			60,
			"Create First Spell"
		) then
			SpellConfig.add_spell()
			SpellConfig.save_to_file()
		end
		return
	end

	local spell = SpellConfig.get_active_spell()
	local old_name = spell.name
	local old_count = spell.projectile_count
	local old_speed = spell.projectile_speed
	local old_size = spell.projectile_size
	local old_ricochet = spell.ricochet
	local old_stagger_delay = spell.stagger_delay

	local panel_width = 520
	local panel_x = (engine.screen_width() - panel_width) * 0.5
	local slider_width = 360
	local controls_x = (engine.screen_width() - slider_width) * 0.5
	local name_y = 150
	local top_y = 220

	engine.text("Spell Editor", 44, 0, editor_y(25), engine.screen_width(), 50)
	
	engine.text(
		"Slot " .. tostring(SpellConfig.active_slot) .. " / " .. tostring(SpellConfig.get_spell_count()) .. "  |  " .. SpellConfig.get_slot_range_hint(),
		20,
		0,
		editor_y(75),
		engine.screen_width(),
		24
	)

	local entity_label = spell.entity_id and ("Entity #" .. tostring(spell.entity_id)) or "Entity pending"
	engine.text(entity_label, 18, controls_x, editor_y(110), slider_width, 24)

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

	-- Projectile Behavior Section
	engine.text("PROJECTILE BEHAVIOR", 22, controls_x, editor_y(top_y), slider_width, 24)
	
	local row = 1
	local row_step = 62
	
	spell.damage = math.floor(engine.slider(
		"spell_damage",
		spell.damage,
		1,
		200,
		controls_x,
		editor_y(top_y + (row * row_step)),
		slider_width,
		"Damage"
	) + 0.5)
	spell.projectile_damage = spell.projectile_damage or {}
	spell.projectile_damage.value = spell.damage

	row = row + 1
	spell.projectile_count = math.floor(engine.slider(
		"spell_projectile_count",
		spell.projectile_count,
		1,
		12,
		controls_x,
		editor_y(top_y + (row * row_step)),
		slider_width,
		"Count"
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
		"Speed"
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
		"Size"
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
	spell.projectile_ricochet = spell.projectile_ricochet or {}
	spell.projectile_ricochet.bounces_left = spell.ricochet and 1 or 0

	-- Color Section
	row = row + 2
	engine.text("PROJECTILE COLOR", 22, controls_x, editor_y(top_y + (row * row_step) - 10), slider_width, 24)
	
	row = row + 1
	spell.color = spell.color or { r = 1.0, g = 0.82, b = 0.33 }
	
	-- Draw color preview swatch
	local swatch_size = 30
	local swatch_x = controls_x + slider_width - swatch_size - 10
	local swatch_y = editor_y(top_y + (row * row_step) - 5)
	engine.draw_rect(swatch_x, swatch_y, swatch_size, swatch_size, 
		math.floor(spell.color.r * 255), 
		math.floor(spell.color.g * 255), 
		math.floor(spell.color.b * 255), 
		255)
	
	spell.color.r = engine.slider(
		"spell_color_r",
		spell.color.r,
		0.0,
		1.0,
		controls_x,
		editor_y(top_y + (row * row_step)),
		slider_width - swatch_size - 20,
		"R"
	)
	
	row = row + 1
	spell.color.g = engine.slider(
		"spell_color_g",
		spell.color.g,
		0.0,
		1.0,
		controls_x,
		editor_y(top_y + (row * row_step)),
		slider_width - swatch_size - 20,
		"G"
	)
	
	row = row + 1
	spell.color.b = engine.slider(
		"spell_color_b",
		spell.color.b,
		0.0,
		1.0,
		controls_x,
		editor_y(top_y + (row * row_step)),
		slider_width - swatch_size - 20,
		"B"
	)
	spell.projectile_color = { r = spell.color.r, g = spell.color.g, b = spell.color.b }

	-- Timing Section
	row = row + 2
	engine.text("SPELL TIMING", 22, controls_x, editor_y(top_y + (row * row_step) - 10), slider_width, 24)
	
	row = row + 1
	spell.stagger_delay = engine.slider(
		"spell_stagger_delay",
		spell.stagger_delay,
		0.0,
		0.5,
		controls_x,
		editor_y(top_y + (row * row_step)),
		slider_width,
		"Stagger (sec)"
	)

	SpellConfig.refresh_spell_components(spell)
	SpellConfig.recalculate_damage(spell)

	if old_name ~= spell.name or
		old_count ~= spell.projectile_count or
		old_speed ~= spell.projectile_speed or
		old_size ~= spell.projectile_size or
		old_ricochet ~= spell.ricochet or
		old_stagger_delay ~= spell.stagger_delay then
		SpellConfig.sync_active_spell_to_entity()
		SpellConfig.save_to_file()
	end

	if spell.entity_id then
		local updated = SpellConfig.sync_active_spell_to_entity()
		if updated then
			SpellConfig.save_to_file()
		end
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

function update_editor()
	intro_timer = intro_timer + engine.delta_time()
	if SpellConfig.handle_slot_input() then
		SpellConfig.save_to_file()
	end
	draw_spell_slot_buttons()
	draw_spell_editor()
	return_button()
end