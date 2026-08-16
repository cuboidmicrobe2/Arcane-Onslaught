SpellConfig = SpellConfig or {}

local DEFAULTS = {
    projectile_count = 1,
    projectile_speed = 320,
    projectile_size = 10,
    ricochet = false,
    stagger_delay = 0.0
}

-- Color for each spell slot (RGB normalized 0-1 for Lua, converted to 0-255 in C++)
local SPELL_COLORS = {
    { r = 1.0, g = 0.82, b = 0.33 },
    { r = 0.2, g = 0.8, b = 1.0 },
    { r = 1.0, g = 0.2, b = 0.8 },
    { r = 0.2, g = 1.0, b = 0.4 },
    { r = 1.0, g = 0.5, b = 0.2 },
}

SpellConfig.save_path = "scripts/spell_saved.lua"

local function parse_slot_count_from_text(text)
    local n = tonumber(tostring(text or ""):match("(%d+)"))
    if not n then
        return 0
    end
    return math.max(0, math.min(9, math.floor(n + 0.5)))
end

SpellConfig.slot_title = SpellConfig.slot_title or "Spell Slots: 0"
SpellConfig.slot_count = SpellConfig.slot_count or parse_slot_count_from_text(SpellConfig.slot_title)
SpellConfig.active_slot = SpellConfig.active_slot or 0
SpellConfig.spells = SpellConfig.spells or {}

local function default_spell_name(slot)
    return "Spell " .. tostring(slot)
end

local function new_spell(slot)
    local index = (tonumber(slot) or 1)
    local color = SPELL_COLORS[((index - 1) % #SPELL_COLORS) + 1] or SPELL_COLORS[1]
    return {
        name = default_spell_name(index),
        projectile_count = DEFAULTS.projectile_count,
        projectile_speed = DEFAULTS.projectile_speed,
        projectile_size = DEFAULTS.projectile_size,
        ricochet = DEFAULTS.ricochet,
        damage = 18,
        stagger_delay = DEFAULTS.stagger_delay,
        color = color
    }
end

local function ensure_spell_slots()
    SpellConfig.spells = SpellConfig.spells or {}
    local clean = {}
    for i = 1, #SpellConfig.spells do
        if SpellConfig.spells[i] ~= nil then
            clean[i] = SpellConfig.spells[i]
        end
    end
    SpellConfig.spells = clean
    if SpellConfig.spells[SpellConfig.active_slot] == nil and SpellConfig.active_slot > 0 then
        SpellConfig.active_slot = #SpellConfig.spells
    end
end

function SpellConfig.get_spell_count()
    return #SpellConfig.spells
end

function SpellConfig.add_spell()
    local next_slot = SpellConfig.get_spell_count() + 1
    local spell = new_spell(next_slot)
    spell.spell_tag = true
    spell.projectile_damage = { value = spell.damage }
    spell.projectile_ricochet = { bounces_left = spell.ricochet and 1 or 0 }
    spell.projectile_color = { r = spell.color.r, g = spell.color.g, b = spell.color.b }

    if engine and engine.spawn_entity then
        spell.entity_id = engine.spawn_entity(spell)
    end

    SpellConfig.spells[next_slot] = spell
    SpellConfig.slot_count = next_slot
    SpellConfig.active_slot = next_slot
    SpellConfig.recalculate_damage(spell)
    return spell
end

function SpellConfig.refresh_spell_components(spell)
    if not spell then
        return
    end

    spell.spell_tag = true
    spell.projectile_damage = spell.projectile_damage or {}
    spell.projectile_damage.value = tonumber(spell.damage) or tonumber(spell.projectile_damage.value) or 1
    spell.projectile_ricochet = spell.projectile_ricochet or {}
    spell.projectile_ricochet.bounces_left = spell.ricochet and 1 or 0
    spell.projectile_color = spell.projectile_color or {}
    if spell.color then
        spell.projectile_color.r = tonumber(spell.color.r) or tonumber(spell.projectile_color.r) or 1.0
        spell.projectile_color.g = tonumber(spell.color.g) or tonumber(spell.projectile_color.g) or 0.82
        spell.projectile_color.b = tonumber(spell.color.b) or tonumber(spell.projectile_color.b) or 0.33
    else
        spell.projectile_color.r = tonumber(spell.projectile_color.r) or 1.0
        spell.projectile_color.g = tonumber(spell.projectile_color.g) or 0.82
        spell.projectile_color.b = tonumber(spell.projectile_color.b) or 0.33
    end
end

function SpellConfig.sync_active_spell_to_entity()
    local spell = SpellConfig.get_active_spell()
    if not spell or not spell.entity_id then
        return false
    end

    SpellConfig.refresh_spell_components(spell)
    if engine and engine.update_entity_data then
        return engine.update_entity_data(spell.entity_id, spell)
    end

    return false
end

function SpellConfig.set_slot_title(text)
    SpellConfig.slot_title = tostring(text or "Spell Slots: 0")
    SpellConfig.slot_count = parse_slot_count_from_text(SpellConfig.slot_title)
    if SpellConfig.get_spell_count() == 0 then
        SpellConfig.active_slot = 0
    end
end

function SpellConfig.get_slot_range_hint()
    local count = SpellConfig.get_spell_count()
    if count == 0 then
        return "0"
    end
    return "1-" .. tostring(count)
end

local function clamp_spell(spell, slot)
    local name = tostring(spell.name or "")
    name = string.gsub(name, "^%s+", "")
    name = string.gsub(name, "%s+$", "")
    if name == "" then
        name = default_spell_name(slot or 1)
    end
    if #name > 24 then
        name = string.sub(name, 1, 24)
    end
    spell.name = name

    spell.projectile_count = tonumber(spell.projectile_count) or DEFAULTS.projectile_count
    spell.projectile_speed = tonumber(spell.projectile_speed) or DEFAULTS.projectile_speed
    spell.projectile_size = tonumber(spell.projectile_size) or DEFAULTS.projectile_size
    spell.ricochet = (spell.ricochet == true)

    spell.projectile_count = math.max(1, math.min(12, math.floor(spell.projectile_count + 0.5)))
    spell.projectile_speed = math.max(120, math.min(900, math.floor(spell.projectile_speed + 0.5)))
    spell.projectile_size = math.max(4, math.min(36, math.floor(spell.projectile_size + 0.5)))
end

function SpellConfig.get_active_spell()
    if SpellConfig.get_spell_count() == 0 then
        return SpellConfig.add_spell()
    end

    local spell = SpellConfig.spells[SpellConfig.active_slot]
    if not spell then
        for i = 1, SpellConfig.get_spell_count() do
            if SpellConfig.spells[i] then
                SpellConfig.active_slot = i
                return SpellConfig.spells[i]
            end
        end
        return SpellConfig.add_spell()
    end
    return spell
end

function SpellConfig.set_active_slot(slot)
    local count = SpellConfig.get_spell_count()
    if count == 0 then
        SpellConfig.active_slot = 0
        return
    end

    local clamped = math.max(1, math.min(count, math.floor(slot + 0.5)))
    SpellConfig.active_slot = clamped
end

function SpellConfig.handle_slot_input()
    local changed = false
    local count = SpellConfig.get_spell_count()
    if count == 0 then
        return false
    end

    local number_slot = engine.number_key_pressed()
    if number_slot >= 1 and number_slot <= count and number_slot ~= SpellConfig.active_slot then
        SpellConfig.set_active_slot(number_slot)
        changed = true
    end

    if engine.next_spell_pressed() then
        local next_slot = (SpellConfig.active_slot % count) + 1
        SpellConfig.set_active_slot(next_slot)
        changed = true
    end

    if engine.prev_spell_pressed() then
        local prev_slot = SpellConfig.active_slot - 1
        if prev_slot < 1 then
            prev_slot = count
        end
        SpellConfig.set_active_slot(prev_slot)
        changed = true
    end

    return changed
end

function SpellConfig.recalculate_damage(spell)
    if not spell then
        spell = SpellConfig.get_active_spell()
    end
    local base = 14.0
    local count_mult = 1.0 / spell.projectile_count
    local speed_mult = 0.65 + (spell.projectile_speed / 700.0)
    local size_mult = 1.0 / (1.0 + (spell.projectile_size - 10.0) / 20.0)
    local ricochet_mult = spell.ricochet and 1.18 or 1.0

    local value = base * count_mult * speed_mult * size_mult * ricochet_mult
    spell.damage = math.max(1, math.floor(value + 0.5))
end

function SpellConfig.load_from_file()
    ensure_spell_slots()

    local chunk = loadfile(SpellConfig.save_path)
    if not chunk then
        SpellConfig.spells = {}
        SpellConfig.slot_count = 0
        SpellConfig.active_slot = 0
        return false
    end

    local ok, data = pcall(chunk)
    if not ok or type(data) ~= "table" then
        SpellConfig.spells = {}
        SpellConfig.slot_count = 0
        SpellConfig.active_slot = 0
        return false
    end

    SpellConfig.spells = {}
    if type(data.slots) == "table" then
        for i = 1, #data.slots do
            local source = data.slots[i] or {}
            local spell = new_spell(i)
            spell.name = source.name or spell.name
            spell.projectile_count = source.projectile_count or DEFAULTS.projectile_count
            spell.projectile_speed = source.projectile_speed or DEFAULTS.projectile_speed
            spell.projectile_size = source.projectile_size or DEFAULTS.projectile_size
            spell.ricochet = (source.ricochet == true)
            clamp_spell(spell, i)
            SpellConfig.recalculate_damage(spell)
            SpellConfig.spells[i] = spell
        end
        SpellConfig.slot_count = #SpellConfig.spells
        if SpellConfig.slot_count > 0 then
            SpellConfig.active_slot = tonumber(data.active_slot) or 1
            if SpellConfig.active_slot > SpellConfig.slot_count then
                SpellConfig.active_slot = SpellConfig.slot_count
            end
        else
            SpellConfig.active_slot = 0
        end
    else
        local spell = new_spell(1)
        spell.projectile_count = data.projectile_count or DEFAULTS.projectile_count
        spell.projectile_speed = data.projectile_speed or DEFAULTS.projectile_speed
        spell.projectile_size = data.projectile_size or DEFAULTS.projectile_size
        spell.ricochet = (data.ricochet == true)
        spell.name = data.name or spell.name
        clamp_spell(spell, 1)
        SpellConfig.recalculate_damage(spell)
        SpellConfig.spells[1] = spell
        SpellConfig.slot_count = 1
        SpellConfig.active_slot = 1
    end

    return true
end

function SpellConfig.save_to_file()
    ensure_spell_slots()

    local file = io.open(SpellConfig.save_path, "w")
    if not file then
        return false
    end

    file:write("return {\n")
    file:write("    active_slot = ", tostring(SpellConfig.active_slot), ",\n")
    file:write("    slots = {\n")
    for i = 1, SpellConfig.get_spell_count() do
        local spell = SpellConfig.spells[i] or new_spell(i)
        file:write("        { name = ", string.format("%q", spell.name),
            ", projectile_count = ", tostring(spell.projectile_count),
            ", projectile_speed = ", tostring(spell.projectile_speed),
            ", projectile_size = ", tostring(spell.projectile_size),
            ", ricochet = ", tostring(spell.ricochet), " },\n")
    end
    file:write("    }\n")
    file:write("}\n")
    file:close()
    return true
end

SpellConfig.load_from_file()
