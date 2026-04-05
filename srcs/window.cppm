module;

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string>
#include <array>
#include <vector>
#include <bitset>

export module mka.graphic.window;
export import mka.graphic.context;
export import mka.graphic.mouse;

/// @brief Keep OpenGL viewport in sync with framebuffer size.
void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height) {
    glViewport(0, 0, width, height);
}

export namespace mka::graphic {
	
	/// @brief High-level runtime state of a window instance.
	enum class State {
		Inited, Running, Stopped, Terminated
	};

	//-----
	enum class KeyState {
		Pressed, Released
	};

	enum class Key {
		Space, Apostrophe, Comma, Minus, Period, Slash,
		Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
		Semicolon, Equal,
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		LBracket, Backslash, RBracket, GraveAccent, /**/ /**/
		Escape, Enter, Tab, Backspace, Insert, Delete, 
		Right, Left, Down, Up, PageUp, PageDown, Home, End, 
		CapsLock, ScrollLock, NumLock, PrintScreen, Pause,
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, F17,
		F18, F19, F20, F21, F22, F23, F24, F25,
		Numpad0, Numpad1, Numpad2, Numpad3, Numpad4, Numpad5, Numpad6, 
		Numpad7, Numpad8, Numpad9, NumpadDecimal, NumpadDivide, NumpadMultiply,
		NumpadSubtract, NumpadAdd, NumpadEnter, NumpadEqual,
		LeftShift, LeftCtrl, LeftAlt, LeftSuper, 
		RightShift, RightCtrl, RightAlt, RightSuper,
		Menu, Unknown
	};

	class KeyboardEvent {
		private:
			std::bitset<static_cast<size_t>(Key::Unknown)> states;

		public:
			bool isPressed(Key key) const {
				return states.test(static_cast<size_t>(key));
			}

			bool isReleased(Key key) const {
				return !states.test(static_cast<size_t>(key));
			}

			void set(Key key, KeyState state) {
				const size_t idx = static_cast<size_t>(key);

				if(state == KeyState::Pressed) {
					states.set(idx);
				} else {
					states.reset(idx);
				}
			}
			
			std::vector<Key> pressedKeys() const {
				std::vector<Key> keys;
				keys.reserve(states.count());
				for (size_t i = 0; i < states.size(); ++i) {
					if (states[i]) keys.emplace_back(static_cast<Key>(i));
				}
				return keys;
			}
	};

	//----
	/**
	 * @brief Base window abstraction hosting the render loop.
	 *
	 * Derived classes only implement `render()` while this class manages:
	 * - GLFW init/teardown
	 * - API-specific window hints
	 * - Context setup and buffer presentation
	 * - Projection updates on resize
	 */
	class Window {
	
		public:
			Window(size_t width, size_t height, const std::string& name, std::unique_ptr<Context> ctx) : size(width,height), name(name), ctx(std::move(ctx)), window(nullptr) {
				if(!glfwInit()) {
					state = State::Terminated;
					return;
				}

				state = State::Inited;
		
				glfwToKey.fill(Key::Unknown);
				initKeyMap();

				switch (this->ctx->getAPI()) {
					case API::OpenGL:
						glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
						glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
						glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
						glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
						break;

					case API::Vulkan:
						glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
						break;

					case API::None:
					
					default:
						glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
						break;
				}

				window = glfwCreateWindow(size.x, size.y, this->name.c_str(), nullptr, nullptr);

				if(!window) {
					state = State::Terminated;
					glfwTerminate();
					return;
				}
				
				glfwSetWindowUserPointer(window, this);
				glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
				
				glfwSetWindowSizeCallback(window, 
					[](GLFWwindow* win, int w, int h) {
						Window* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
						self->orthographicProjection = glm::ortho(0.0f, float(w), float(h), 0.0f, -1.0f, 1.0f);
						self->size = glm::vec2(w, h);
					}
				);

				glfwSetCursorPosCallback(window,
					[](GLFWwindow* win, double x, double y) {	
						Window* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
						self->mouseEvent.position = glm::vec2(x, y);
					}
				);
				
				glfwSetKeyCallback(window, 
					[](GLFWwindow* win, int key, int /*scancode*/, int action, int /*mods*/) {

						if(key < 0 || key >= KEY_COUNT) {
							return;
						}

						if(action != GLFW_PRESS && action != GLFW_RELEASE) {
							return;
						}

						Window* self = static_cast<Window*>(glfwGetWindowUserPointer(win));

						const Key mappedKey = self->glfwToKey[static_cast<size_t>(key)];
						
						if(mappedKey == Key::Unknown) {
							return;
						}

						self->keyboardEvent.set(mappedKey, (action == GLFW_RELEASE) ? KeyState::Released : KeyState::Pressed);
					}
				);
				
				glfwSetScrollCallback(window,
					[](GLFWwindow* win, double xOffset, double yOffset) {
						Window* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
						self->mouseEvent.scroll = glm::vec2(xOffset, yOffset);
						
						if(yOffset > 0) self->mouseEvent.set(MouseButton::Middle, MouseState::ScrollUp);
						if(yOffset < 0) self->mouseEvent.set(MouseButton::Middle, MouseState::ScrollDown);
					}
				);

				glfwSetMouseButtonCallback(window, 
					[](GLFWwindow* win, int button, int action, int /*mods*/) {
				    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(win));

					MouseState state = (action == GLFW_PRESS) ? MouseState::Pressed : MouseState::Released;
					MouseButton btn;

					switch(button) {
						case GLFW_MOUSE_BUTTON_LEFT:   btn = MouseButton::Left; break;
						case GLFW_MOUSE_BUTTON_MIDDLE: btn = MouseButton::Middle; break;
						case GLFW_MOUSE_BUTTON_RIGHT:  btn = MouseButton::Right; break;
					}

					self->mouseEvent.set(btn, state);
				});

				if(!this->ctx || !this->ctx->init(window)) {
					state = State::Terminated;
					return;
				}
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			
			virtual void render(const glm::vec2& size, const MouseEvent& mouse, const KeyboardEvent& keyboard) = 0;

			/// @brief Projection updated whenever the window size changes.
			const glm::mat4& getOrthographicProjection() const {
				return orthographicProjection;
			}

			/// @brief Run the blocking frame loop until close.
			int run() {
				if(state != State::Inited) return -1;

				state = State::Running;

				while(!glfwWindowShouldClose(window)) {	
					glfwPollEvents();

					ctx->makeCurrent();

					render(size, mouseEvent, keyboardEvent);

					ctx->swapBuffers();
				}

				state = State::Stopped;
				return 0;
			}

			~Window() {
				if(window) {
					glfwDestroyWindow(window);
				}
				glfwTerminate();
			}

			const State& getState() {
				return state;
			}

		private:
		void initKeyMap() {

			glfwToKey[GLFW_KEY_SPACE] = Key::Space;
			glfwToKey[GLFW_KEY_APOSTROPHE] = Key::Apostrophe;
			glfwToKey[GLFW_KEY_COMMA] = Key::Comma;
			glfwToKey[GLFW_KEY_MINUS] = Key::Minus;
			glfwToKey[GLFW_KEY_PERIOD] = Key::Period;
			glfwToKey[GLFW_KEY_SLASH] = Key::Slash;
			
			glfwToKey[GLFW_KEY_0] = Key::Num0;
			glfwToKey[GLFW_KEY_1] = Key::Num1;
			glfwToKey[GLFW_KEY_2] = Key::Num2;
			glfwToKey[GLFW_KEY_3] = Key::Num3;
			glfwToKey[GLFW_KEY_4] = Key::Num4;
			glfwToKey[GLFW_KEY_5] = Key::Num5;
			glfwToKey[GLFW_KEY_6] = Key::Num6;
			glfwToKey[GLFW_KEY_7] = Key::Num7;
			glfwToKey[GLFW_KEY_8] = Key::Num8;
			glfwToKey[GLFW_KEY_9] = Key::Num9;

			glfwToKey[GLFW_KEY_SEMICOLON] = Key::Semicolon;
			glfwToKey[GLFW_KEY_EQUAL] = Key::Equal;
			
			glfwToKey[GLFW_KEY_A] = Key::A;
			glfwToKey[GLFW_KEY_B] = Key::B;
			glfwToKey[GLFW_KEY_C] = Key::C;
			glfwToKey[GLFW_KEY_D] = Key::D;
			glfwToKey[GLFW_KEY_E] = Key::E;
			glfwToKey[GLFW_KEY_F] = Key::F;
			glfwToKey[GLFW_KEY_G] = Key::G;
			glfwToKey[GLFW_KEY_H] = Key::H;
			glfwToKey[GLFW_KEY_I] = Key::I;
			glfwToKey[GLFW_KEY_J] = Key::J;
			glfwToKey[GLFW_KEY_K] = Key::K;
			glfwToKey[GLFW_KEY_L] = Key::L;
			glfwToKey[GLFW_KEY_M] = Key::M;
			glfwToKey[GLFW_KEY_N] = Key::N;
			glfwToKey[GLFW_KEY_O] = Key::O;
			glfwToKey[GLFW_KEY_P] = Key::P;
			glfwToKey[GLFW_KEY_Q] = Key::Q;
			glfwToKey[GLFW_KEY_R] = Key::R;
			glfwToKey[GLFW_KEY_S] = Key::S;
			glfwToKey[GLFW_KEY_T] = Key::T;
			glfwToKey[GLFW_KEY_U] = Key::U;
			glfwToKey[GLFW_KEY_V] = Key::V;
			glfwToKey[GLFW_KEY_W] = Key::W;
			glfwToKey[GLFW_KEY_X] = Key::X;
			glfwToKey[GLFW_KEY_Y] = Key::Y;
			glfwToKey[GLFW_KEY_Z] = Key::Z;

			glfwToKey[GLFW_KEY_LEFT_BRACKET] = Key::LBracket;
			glfwToKey[GLFW_KEY_BACKSLASH] = Key::Backslash;
			glfwToKey[GLFW_KEY_RIGHT_BRACKET] = Key::RBracket;
			glfwToKey[GLFW_KEY_GRAVE_ACCENT] = Key::GraveAccent;
			/**/
			/**/
			glfwToKey[GLFW_KEY_ESCAPE] = Key::Escape;
			glfwToKey[GLFW_KEY_ENTER] = Key::Enter;
			glfwToKey[GLFW_KEY_TAB] = Key::Tab;
			glfwToKey[GLFW_KEY_BACKSPACE] = Key::Backspace;
			glfwToKey[GLFW_KEY_INSERT] = Key::Insert;
			glfwToKey[GLFW_KEY_DELETE] = Key::Delete;
			
			glfwToKey[GLFW_KEY_RIGHT] = Key::Right;
			glfwToKey[GLFW_KEY_LEFT]  = Key::Left;
			glfwToKey[GLFW_KEY_DOWN]  = Key::Down;
			glfwToKey[GLFW_KEY_UP]    = Key::Up;
			
			glfwToKey[GLFW_KEY_PAGE_UP] = Key::PageUp;
			glfwToKey[GLFW_KEY_PAGE_DOWN] = Key::PageDown;
			glfwToKey[GLFW_KEY_HOME] = Key::Home;
			glfwToKey[GLFW_KEY_END] = Key::End;
			
			glfwToKey[GLFW_KEY_CAPS_LOCK] = Key::CapsLock;
			glfwToKey[GLFW_KEY_SCROLL_LOCK] = Key::ScrollLock;
			glfwToKey[GLFW_KEY_NUM_LOCK] = Key::NumLock;
			glfwToKey[GLFW_KEY_PRINT_SCREEN] = Key::PrintScreen;
			glfwToKey[GLFW_KEY_PAUSE] = Key::Pause;
			
			glfwToKey[GLFW_KEY_F1] = Key::F1;
			glfwToKey[GLFW_KEY_F2] = Key::F2;
			glfwToKey[GLFW_KEY_F3] = Key::F3;
			glfwToKey[GLFW_KEY_F4] = Key::F4;
			glfwToKey[GLFW_KEY_F5] = Key::F5;
			glfwToKey[GLFW_KEY_F6] = Key::F6;
			glfwToKey[GLFW_KEY_F7] = Key::F7;
			glfwToKey[GLFW_KEY_F8] = Key::F8;
			glfwToKey[GLFW_KEY_F9] = Key::F9;
			glfwToKey[GLFW_KEY_F10] =Key::F10;
			glfwToKey[GLFW_KEY_F11] =Key::F11;
			glfwToKey[GLFW_KEY_F12] =Key::F12;
			glfwToKey[GLFW_KEY_F13] =Key::F13;
			glfwToKey[GLFW_KEY_F14] =Key::F14;
			glfwToKey[GLFW_KEY_F15] =Key::F15;
			glfwToKey[GLFW_KEY_F16] =Key::F16;
			glfwToKey[GLFW_KEY_F17] =Key::F17;
			glfwToKey[GLFW_KEY_F18] =Key::F18;
			glfwToKey[GLFW_KEY_F19] =Key::F19;
			glfwToKey[GLFW_KEY_F20] =Key::F20;
			glfwToKey[GLFW_KEY_F21] =Key::F21;
			glfwToKey[GLFW_KEY_F22] =Key::F22;
			glfwToKey[GLFW_KEY_F23] =Key::F23;
			glfwToKey[GLFW_KEY_F24] =Key::F24;
			glfwToKey[GLFW_KEY_F25] =Key::F25;

			glfwToKey[GLFW_KEY_KP_0] = Key::Numpad0;
			glfwToKey[GLFW_KEY_KP_1] = Key::Numpad1;
			glfwToKey[GLFW_KEY_KP_2] = Key::Numpad2;
			glfwToKey[GLFW_KEY_KP_3] = Key::Numpad3;
			glfwToKey[GLFW_KEY_KP_4] = Key::Numpad4;
			glfwToKey[GLFW_KEY_KP_5] = Key::Numpad5;
			glfwToKey[GLFW_KEY_KP_6] = Key::Numpad6;
			glfwToKey[GLFW_KEY_KP_7] = Key::Numpad7;
			glfwToKey[GLFW_KEY_KP_8] = Key::Numpad8;
			glfwToKey[GLFW_KEY_KP_9] = Key::Numpad9;

			glfwToKey[GLFW_KEY_KP_DECIMAL] = Key::NumpadDecimal;
			glfwToKey[GLFW_KEY_KP_DIVIDE] = Key::NumpadDivide;
			glfwToKey[GLFW_KEY_KP_MULTIPLY] = Key::NumpadMultiply;
			glfwToKey[GLFW_KEY_KP_SUBTRACT] = Key::NumpadSubtract;
			glfwToKey[GLFW_KEY_KP_ADD] = Key::NumpadAdd;
			glfwToKey[GLFW_KEY_KP_ENTER] = Key::NumpadEnter;	
			
			glfwToKey[GLFW_KEY_LEFT_SHIFT] = Key::LeftShift;
			glfwToKey[GLFW_KEY_LEFT_CONTROL] = Key::LeftCtrl;
			glfwToKey[GLFW_KEY_LEFT_ALT] = Key::LeftAlt;
			glfwToKey[GLFW_KEY_LEFT_SUPER] = Key::LeftSuper;
			
			glfwToKey[GLFW_KEY_RIGHT_SHIFT] = Key::RightShift;
			glfwToKey[GLFW_KEY_RIGHT_CONTROL] = Key::RightCtrl;
			glfwToKey[GLFW_KEY_RIGHT_ALT] = Key::RightAlt;
			glfwToKey[GLFW_KEY_RIGHT_SUPER] = Key::RightSuper;
			
			glfwToKey[GLFW_KEY_MENU] = Key::Menu;
		}
		private:

		glm::vec2 size = {0.0, 0.0};
		std::string	name;
		std::unique_ptr<Context> ctx;
		State state = State::Terminated;
		GLFWwindow* window = nullptr;
		glm::mat4 orthographicProjection;
		MouseEvent mouseEvent;
		KeyboardEvent keyboardEvent;

		static constexpr int KEY_COUNT = GLFW_KEY_LAST + 1;
		std::array<Key, KEY_COUNT> glfwToKey;
	};
}
