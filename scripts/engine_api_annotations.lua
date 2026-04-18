---@meta

---@class EngineAPI
engine = engine or {}

---Draw a clickable button.
---Returns `true` only on the frame the button is pressed.
---@param id string Unique control id used to preserve widget interaction state.
---@param x number Left position in pixels.
---@param y number Top position in pixels.
---@param w number Width in pixels.
---@param h number Height in pixels.
---@param text string Text rendered inside the button.
---@return boolean pressed
function engine.button(id, x, y, w, h, text) end

---Draw text inside a rectangle.
---Returns `true` when drawing succeeds.
---@param text string Text to render.
---@param fontSize integer Font size in pixels.
---@param x number Left position in pixels.
---@param y number Top position in pixels.
---@param w number Width of the text bounds.
---@param h number Height of the text bounds.
---@return boolean drawn
function engine.text(text, fontSize, x, y, w, h) end

---Draw a horizontal slider and return the updated value.
---@param id string Unique control id.
---@param value number Current value.
---@param minValue number Minimum slider value.
---@param maxValue number Maximum slider value.
---@param x number Left position in pixels.
---@param y number Top position in pixels.
---@param w number Slider width in pixels.
---@param label? string Optional label; defaults to `id` when omitted.
---@return number value
function engine.slider(id, value, minValue, maxValue, x, y, w, label) end

---Draw a text input field and return the updated text.
---@param id string Unique control id.
---@param value string Current field value.
---@param maxChars integer Maximum number of characters accepted.
---@param x number Left position in pixels.
---@param y number Top position in pixels.
---@param w number Field width in pixels.
---@param h number Field height in pixels.
---@param label? string Optional label; defaults to `id` when omitted.
---@return string value
function engine.text_field(id, value, maxChars, x, y, w, h, label) end

---Draw a checkbox and return the updated checked state.
---@param id string Unique control id.
---@param checked boolean Current checked state.
---@param x number Left position in pixels.
---@param y number Top position in pixels.
---@param size number Box size in pixels.
---@param label? string Optional label; defaults to `id` when omitted.
---@return boolean checked
function engine.checkbox(id, checked, x, y, size, label) end

---Switch the game to the main menu scene.
function engine.main_menu() end

---Switch the game to the gameplay scene.
function engine.start_game() end

---Switch the game to the editor scene.
function engine.open_editor() end

---Request the game to quit.
function engine.quit_game() end

---Run one gameplay frame while staying in the current scene.
function engine.run_game() end

---Apply spell configuration values to the active game state.
---@param projectileCount integer
---@param projectileSpeed number
---@param projectileSize number
---@param ricochet boolean
---@param damage integer
function engine.set_spell_config(projectileCount, projectileSpeed, projectileSize, ricochet, damage) end

---Returns 1 to 5 when number keys 1..5 are pressed; otherwise 0.
---@return integer key
function engine.number_key_pressed() end

---Returns `true` when the next-spell input is pressed this frame.
---@return boolean pressed
function engine.next_spell_pressed() end

---Returns `true` when the previous-spell input is pressed this frame.
---@return boolean pressed
function engine.prev_spell_pressed() end

---Get current screen width in pixels.
---@return number width
function engine.screen_width() end

---Get current screen height in pixels.
---@return number height
function engine.screen_height() end

---Frame delta time in seconds.
---@return number seconds
function engine.delta_time() end

---Legacy global API aliases kept for backward compatibility.
---@deprecated Use `engine.button` instead.
button = engine.button
---@deprecated Use `engine.text` instead.
text = engine.text
---@deprecated Use `engine.slider` instead.
slider = engine.slider
---@deprecated Use `engine.text_field` instead.
text_field = engine.text_field
---@deprecated Use `engine.checkbox` instead.
checkbox = engine.checkbox
---@deprecated Use `engine.main_menu` instead.
main_menu = engine.main_menu
---@deprecated Use `engine.start_game` instead.
start_game = engine.start_game
---@deprecated Use `engine.open_editor` instead.
open_editor = engine.open_editor
---@deprecated Use `engine.quit_game` instead.
quit_game = engine.quit_game
---@deprecated Use `engine.run_game` instead.
run_game = engine.run_game
---@deprecated Use `engine.set_spell_config` instead.
set_spell_config = engine.set_spell_config
---@deprecated Use `engine.number_key_pressed` instead.
number_key_pressed = engine.number_key_pressed
---@deprecated Use `engine.next_spell_pressed` instead.
next_spell_pressed = engine.next_spell_pressed
---@deprecated Use `engine.prev_spell_pressed` instead.
prev_spell_pressed = engine.prev_spell_pressed
---@deprecated Use `engine.screen_width` instead.
screen_width = engine.screen_width
---@deprecated Use `engine.screen_height` instead.
screen_height = engine.screen_height
---@deprecated Use `engine.delta_time` instead.
delta_time = engine.delta_time
