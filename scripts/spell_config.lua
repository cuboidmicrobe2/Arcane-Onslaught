SpellConfig = SpellConfig or {}

local DEFAULTS = {
    projectile_count = 1,
    projectile_speed = 320,
    projectile_size = 10,
    ricochet = false
}

SpellConfig.save_path = "scripts/spell_saved.lua"
SpellConfig.slot_count = 5
SpellConfig.active_slot = SpellConfig.active_slot or 1

local function default_spell_name(slot)
    return "Spell " .. tostring(slot)
end

local function new_spell(slot)
    return {
        name = default_spell_name(slot or 1),
        projectile_count = DEFAULTS.projectile_count,
        projectile_speed = DEFAULTS.projectile_speed,
        projectile_size = DEFAULTS.projectile_size,
        ricochet = DEFAULTS.ricochet,
        damage = 18
    }
end

SpellConfig.spells = SpellConfig.spells or {
    new_spell(1),
    new_spell(2),
    new_spell(3),
    new_spell(4),
    new_spell(5)
}

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
    local spell = SpellConfig.spells[SpellConfig.active_slot]
    if not spell then
        spell = new_spell(SpellConfig.active_slot)
        SpellConfig.spells[SpellConfig.active_slot] = spell
    end
    return spell
end

function SpellConfig.set_active_slot(slot)
    local clamped = math.max(1, math.min(SpellConfig.slot_count, math.floor(slot + 0.5)))
    SpellConfig.active_slot = clamped
end

function SpellConfig.handle_slot_input()
    local changed = false
    local number_slot = engine.number_key_pressed()
    if number_slot >= 1 and number_slot <= SpellConfig.slot_count and number_slot ~= SpellConfig.active_slot then
        SpellConfig.set_active_slot(number_slot)
        changed = true
    end

    if engine.next_spell_pressed() then
        local next_slot = (SpellConfig.active_slot % SpellConfig.slot_count) + 1
        SpellConfig.set_active_slot(next_slot)
        changed = true
    end

    if engine.prev_spell_pressed() then
        local prev_slot = SpellConfig.active_slot - 1
        if prev_slot < 1 then
            prev_slot = SpellConfig.slot_count
        end
        SpellConfig.set_active_slot(prev_slot)
        changed = true
    end

    return changed
end

function SpellConfig.recalculate_damage(spell)
    spell = spell or SpellConfig.get_active_spell()
    local base = 14.0
    local count_mult = 1.0 + ((spell.projectile_count - 1.0) * 0.35)
    local speed_mult = 0.65 + (spell.projectile_speed / 700.0)
    local size_mult = 0.60 + (spell.projectile_size / 24.0)
    local ricochet_mult = spell.ricochet and 1.18 or 1.0

    local value = base * count_mult * speed_mult * size_mult * ricochet_mult
    spell.damage = math.max(1, math.floor(value + 0.5))
end

function SpellConfig.load_from_file()
    local chunk = loadfile(SpellConfig.save_path)
    if not chunk then
        SpellConfig.recalculate_damage()
        return false
    end

    local ok, data = pcall(chunk)
    if not ok or type(data) ~= "table" then
        for i = 1, SpellConfig.slot_count do
            SpellConfig.recalculate_damage(SpellConfig.spells[i])
        end
        return false
    end

    if type(data.slots) == "table" then
        for i = 1, SpellConfig.slot_count do
            local source = data.slots[i] or {}
            local spell = SpellConfig.spells[i] or new_spell()
            spell.projectile_count = source.projectile_count
            spell.projectile_speed = source.projectile_speed
            spell.projectile_size = source.projectile_size
            spell.ricochet = source.ricochet
            spell.name = source.name
            clamp_spell(spell, i)
            SpellConfig.recalculate_damage(spell)
            SpellConfig.spells[i] = spell
        end
        SpellConfig.set_active_slot(tonumber(data.active_slot) or 1)
    else
        -- Backward compatibility with the old single-spell format.
        local spell = SpellConfig.spells[1] or new_spell()
        spell.projectile_count = data.projectile_count
        spell.projectile_speed = data.projectile_speed
        spell.projectile_size = data.projectile_size
        spell.ricochet = data.ricochet
        spell.name = data.name
        clamp_spell(spell, 1)
        SpellConfig.recalculate_damage(spell)
        SpellConfig.spells[1] = spell
        for i = 2, SpellConfig.slot_count do
            local reset = new_spell(i)
            SpellConfig.recalculate_damage(reset)
            SpellConfig.spells[i] = reset
        end
        SpellConfig.set_active_slot(1)
    end

    return true
end

function SpellConfig.save_to_file()
    local file = io.open(SpellConfig.save_path, "w")
    if not file then
        return false
    end

    file:write("return {\n")
    file:write("    active_slot = ", tostring(SpellConfig.active_slot), ",\n")
    file:write("    slots = {\n")
    for i = 1, SpellConfig.slot_count do
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
