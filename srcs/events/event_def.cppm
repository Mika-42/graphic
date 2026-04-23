module;
#include <string_view>

export module mka.graphic.event.definition;

export namespace mka::graphic::event {
    // APP
    inline constexpr std::string_view app_close = "mka.app.close";

    // VIEW
    inline constexpr std::string_view view_add_child = "mka.view.add.child";
    inline constexpr std::string_view view_remove_child = "mka.view.remove.child";
	
    inline constexpr std::string_view view_position_changed = "mka.view.position.changed";
    inline constexpr std::string_view view_size_changed = "mka.view.size.changed";
    inline constexpr std::string_view view_visibility_changed = "mka.view.visibility.changed";

    // MOUSE
    inline constexpr std::string_view mouse_enter = "mka.mouse.enter";
    inline constexpr std::string_view mouse_leave = "mka.mouse.leave";
    inline constexpr std::string_view mouse_move = "mka.mouse.move";

    inline constexpr std::string_view mouse_button_left_up = "mka.mouse.button.left.up";
    inline constexpr std::string_view mouse_button_left_down = "mka.mouse.button.left.down";

    inline constexpr std::string_view mouse_button_middle_up = "mka.mouse.button.middle.up";
    inline constexpr std::string_view mouse_button_middle_down = "mka.mouse.button.middle.down";
    inline constexpr std::string_view mouse_button_scroll_up = "mka.mouse.button.scroll.up";
    inline constexpr std::string_view mouse_button_scroll_down = "mka.mouse.button.scroll.down";
    inline constexpr std::string_view mouse_button_scroll_left = "mka.mouse.button.scroll.left";
    inline constexpr std::string_view mouse_button_scroll_right = "mka.mouse.button.scroll.right";

    inline constexpr std::string_view mouse_button_right_up = "mka.mouse.button.right.up";
    inline constexpr std::string_view mouse_button_right_down = "mka.mouse.button.right.down";
}
