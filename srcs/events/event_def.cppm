module;
#include <string_view>

export module mka.graphic.event.definition;


export namespace mka::graphic::event::app {
    inline constexpr std::string_view close = "mka.app.close";
}

export namespace mka::graphic::event::view {
    inline constexpr std::string_view addChild = "mka.view.add.child";
    inline constexpr std::string_view removeChild = "mka.view.remove.child";
}

export namespace mka::graphic::event::mouse {

    inline constexpr std::string_view enter = "mka.mouse.enter";
    inline constexpr std::string_view leave = "mka.mouse.leave";
    inline constexpr std::string_view move  = "mka.mouse.move";

    namespace button {

        namespace left {
            inline constexpr std::string_view up   = "mka.mouse.button.left.up";
            inline constexpr std::string_view down = "mka.mouse.button.left.down";
        }

        namespace middle {
            inline constexpr std::string_view up          = "mka.mouse.button.middle.up";
            inline constexpr std::string_view down        = "mka.mouse.button.middle.down";
            inline constexpr std::string_view scrollUp    = "mka.mouse.button.scroll.up";
            inline constexpr std::string_view scrollDown  = "mka.mouse.button.scroll.down";
            inline constexpr std::string_view scrollLeft  = "mka.mouse.button.scroll.left";
            inline constexpr std::string_view scrollRight = "mka.mouse.button.scroll.right";
        }

        namespace right {
            inline constexpr std::string_view up   = "mka.mouse.button.right.up";
            inline constexpr std::string_view down = "mka.mouse.button.right.down";
        }
    }
}
