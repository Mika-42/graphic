module;
#include <cstddef>
#include <glm/glm.hpp>

export module mka.graphic.mouse;

export namespace mka::graphic {

	enum class MouseButton {
		Left, Middle, Right,
	};

	enum class MouseState {
		Pressed, Released, ScrollUp, ScrollDown
	};

	class MouseEvent {
		private:
			MouseState states[3] = {
				MouseState::Released,MouseState::Released,MouseState::Released
			};
	
		public:
			bool isPressed(MouseButton btn) const {
				return states[static_cast<size_t>(btn)] == MouseState::Pressed;
			}

			bool isReleased(MouseButton btn) const {
				return states[static_cast<size_t>(btn)] == MouseState::Released;
			}

			bool isScrollUp(MouseButton btn) const {
				return states[static_cast<size_t>(btn)] == MouseState::ScrollUp;
			}

			bool isScrollDown(MouseButton btn) const {
				return states[static_cast<size_t>(btn)] == MouseState::ScrollDown;
			}
		
			void set(MouseButton button, MouseState state) {
				states[static_cast<size_t>(button)] = state;
			}
			
			glm::vec2 scroll = {0.0, 0.0};
			glm::vec2 position = {0.0, 0.0};
	};
}
