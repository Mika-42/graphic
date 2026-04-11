/**
 * @file mouse.cppm
 * @brief Mouse input state model used by the window runtime.
 */
module;
#include <cstddef>
#include <glm/glm.hpp>

export module mka.graphic.mouse;

export namespace mka::graphic {

	/** @brief Mouse buttons supported by the input layer. */
	enum class MouseButton {
		Left, Middle, Right,
	};

	/** @brief Aggregated state for each tracked mouse button. */
	enum class MouseState {
		Pressed, Released, ScrollUp, ScrollDown
	};

	/**
	 * @brief Mutable mouse state snapshot shared with `Window` views.
	 *
	 * The state is updated by GLFW callbacks in `Window` and read from
	 * user-facing `Window::MouseEventView` in the render loop.
	 */
	class MouseEvent {
		private:
			MouseState states[3] = {
				MouseState::Released,MouseState::Released,MouseState::Released
			};
	
		public:
			/** @brief Return true when the button is currently pressed. */
			bool isPressed(MouseButton btn) const {
				return states[static_cast<size_t>(btn)] == MouseState::Pressed;
			}

			/** @brief Return true when the button is currently released. */
			bool isReleased(MouseButton btn) const {
				return states[static_cast<size_t>(btn)] == MouseState::Released;
			}

			/** @brief Return true if the latest wheel event was upward. */
			bool isScrollUp(MouseButton btn) const {
				return states[static_cast<size_t>(btn)] == MouseState::ScrollUp;
			}

			/** @brief Return true if the latest wheel event was downward. */
			bool isScrollDown(MouseButton btn) const {
				return states[static_cast<size_t>(btn)] == MouseState::ScrollDown;
			}
		
			/** @brief Update one button/wheel state. */
			void set(MouseButton button, MouseState state) {
				states[static_cast<size_t>(button)] = state;
			}
			
			/** @brief Last wheel delta from GLFW scroll callback. */
			glm::vec2 scroll = {0.0, 0.0};
			/** @brief Last cursor position in window coordinates. */
			glm::vec2 position = {-99999.0, -99999.0};
	};
}
