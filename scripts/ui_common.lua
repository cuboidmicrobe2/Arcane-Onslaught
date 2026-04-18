UI = UI or {}

function UI.get_centered_x(width)
    return (engine.screen_width() - width) * 0.5
end

function UI.get_default_menu_layout()
    return {
        width = 260,
        button_height = 54,
        spacing = 16
    }
end

function UI.get_menu_button_y(start_y, index, layout)
    return start_y + ((layout.button_height + layout.spacing) * index)
end

function UI.render_menu_button(id, label, start_y, index, layout, on_click)
    local x = UI.get_centered_x(layout.width)
    local y = UI.get_menu_button_y(start_y, index, layout)

    if engine.button(id, x, y, layout.width, layout.button_height, label) then
        on_click()
        return true
    end

    return false
end