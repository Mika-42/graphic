module;

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string>
#include <array>
#include <deque>

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
	enum KeyState {
		Pressed, Released
	};

	enum class KeyboardMod {
		CapsLock, NumLock, Shift, Ctrl, Alt, Super
	};

	enum class KeyboardKey
	{
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
		Tab, Space, Enter, Backspace, Left, Right, Up, Down,
		Escape, Delete, Insert, Home, End, PageUp, PageDown,
		NumpadAdd, NumpadSubtract, NumpadMultiply, NumpadDivide,
		Numpad0, Numpad1, Numpad2, Numpad3, Numpad4, Numpad5, Numpad6, 
		Numpad7, Numpad8, Numpad9, NumpadEnter,
		LeftShift, RightShift,
		LeftCtrl, RightCtrl,
		LeftAlt, RightAlt,
		LeftSuper, RightSuper,
		CapsLock, Unknown
	};

	struct KeyboardEvent {
		KeyState state;
		KeyboardKey key;
		std::vector<KeyboardMod> mod;
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
			Window(size_t width, size_t height, const std::string& name, std::unique_ptr<Context> ctx) : width(width), height(height), name(name), ctx(std::move(ctx)), window(nullptr) {
				if(!glfwInit()) {
					state = State::Terminated;
					return;
				}

				state = State::Inited;
		
				glfwToKey.fill(KeyboardKey::Unknown);
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


				window = glfwCreateWindow(this->width, this->height, this->name.c_str(), nullptr, nullptr);

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
						self->width = w;
						self->height = h;
					}
				);

				glfwSetCursorPosCallback(window,
					[](GLFWwindow* win, double x, double y) {	
						Window* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
						self->mouseEvent.position.x = x;
						self->mouseEvent.position.y = y;
					}
				);
				
				glfwSetKeyCallback(window, 
					[](GLFWwindow* win, int key, int /*scancode*/, int action, int mods) {
						
						Window* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
						
						KeyboardEvent e {};
						e.key = self->glfwToKey[key];

						//----
						if (mods & GLFW_MOD_SHIFT) {
							e.mod.emplace_back(KeyboardMod::Shift);
						}
						if (mods & GLFW_MOD_CONTROL) {
							e.mod.emplace_back(KeyboardMod::Ctrl);
						}
						if (mods & GLFW_MOD_ALT) {
							e.mod.emplace_back(KeyboardMod::Alt);
						}
						if (mods & GLFW_MOD_SUPER) {
							e.mod.emplace_back(KeyboardMod::Super);
						}
						if (mods & GLFW_MOD_CAPS_LOCK) {
							e.mod.emplace_back(KeyboardMod::CapsLock);
						}
						if (mods & GLFW_MOD_NUM_LOCK) {
							e.mod.emplace_back(KeyboardMod::NumLock);
						}

						if (action == GLFW_PRESS) {
							e.state = KeyState::Pressed;
						}
						else if (action == GLFW_RELEASE) {
							e.state = KeyState::Released;
						}

						self->eventQueue.emplace_back(e);
					}
				);
				
				glfwSetScrollCallback(window,
					[](GLFWwindow* win, double xOffset, double yOffset) {
						Window* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
						self->mouseEvent.scroll = glm::vec2(xOffset, yOffset);
						
						if(yOffset > 0) self->mouseEvent.set(MouseBUtton::Middle, MouseState::ScrollUp);
						if(yOffset < 0) self->mouseEvent.set(MouseBUtton::Middle, MouseState::ScrollDown);
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
			
			virtual void render(const MouseEvent& mouse) = 0;

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

					render(mouseEvent);

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

			/// @brief Return current window dimensions in pixels.
			const glm::vec2 getSize() {
				return glm::vec2(width, height);
			}

		private:
		void initKeyMap() {
			glfwToKey[GLFW_KEY_A] = KeyboardKey::A;
			glfwToKey[GLFW_KEY_B] = KeyboardKey::B;
			glfwToKey[GLFW_KEY_C] = KeyboardKey::C;
			glfwToKey[GLFW_KEY_D] = KeyboardKey::D;
			glfwToKey[GLFW_KEY_E] = KeyboardKey::E;
			glfwToKey[GLFW_KEY_F] = KeyboardKey::F;
			glfwToKey[GLFW_KEY_G] = KeyboardKey::G;
			glfwToKey[GLFW_KEY_H] = KeyboardKey::H;
			glfwToKey[GLFW_KEY_I] = KeyboardKey::I;
			glfwToKey[GLFW_KEY_J] = KeyboardKey::J;
			glfwToKey[GLFW_KEY_K] = KeyboardKey::K;
			glfwToKey[GLFW_KEY_L] = KeyboardKey::L;
			glfwToKey[GLFW_KEY_M] = KeyboardKey::M;
			glfwToKey[GLFW_KEY_N] = KeyboardKey::N;
			glfwToKey[GLFW_KEY_O] = KeyboardKey::O;
			glfwToKey[GLFW_KEY_P] = KeyboardKey::P;
			glfwToKey[GLFW_KEY_Q] = KeyboardKey::Q;
			glfwToKey[GLFW_KEY_R] = KeyboardKey::R;
			glfwToKey[GLFW_KEY_S] = KeyboardKey::S;
			glfwToKey[GLFW_KEY_T] = KeyboardKey::T;
			glfwToKey[GLFW_KEY_U] = KeyboardKey::U;
			glfwToKey[GLFW_KEY_V] = KeyboardKey::V;
			glfwToKey[GLFW_KEY_W] = KeyboardKey::W;
			glfwToKey[GLFW_KEY_X] = KeyboardKey::X;
			glfwToKey[GLFW_KEY_Y] = KeyboardKey::Y;
			glfwToKey[GLFW_KEY_Z] = KeyboardKey::Z;

			glfwToKey[GLFW_KEY_0] = KeyboardKey::Num0;
			glfwToKey[GLFW_KEY_1] = KeyboardKey::Num1;
			glfwToKey[GLFW_KEY_2] = KeyboardKey::Num2;
			glfwToKey[GLFW_KEY_3] = KeyboardKey::Num3;
			glfwToKey[GLFW_KEY_4] = KeyboardKey::Num4;
			glfwToKey[GLFW_KEY_5] = KeyboardKey::Num5;
			glfwToKey[GLFW_KEY_6] = KeyboardKey::Num6;
			glfwToKey[GLFW_KEY_7] = KeyboardKey::Num7;
			glfwToKey[GLFW_KEY_8] = KeyboardKey::Num8;
			glfwToKey[GLFW_KEY_9] = KeyboardKey::Num9;

			glfwToKey[GLFW_KEY_F1] = KeyboardKey::F1;
			glfwToKey[GLFW_KEY_F2] = KeyboardKey::F2;
			glfwToKey[GLFW_KEY_F3] = KeyboardKey::F3;
			glfwToKey[GLFW_KEY_F4] = KeyboardKey::F4;
			glfwToKey[GLFW_KEY_F5] = KeyboardKey::F5;
			glfwToKey[GLFW_KEY_F6] = KeyboardKey::F6;
			glfwToKey[GLFW_KEY_F7] = KeyboardKey::F7;
			glfwToKey[GLFW_KEY_F8] = KeyboardKey::F8;
			glfwToKey[GLFW_KEY_F9] = KeyboardKey::F9;
			glfwToKey[GLFW_KEY_F10] = KeyboardKey::F10;
			glfwToKey[GLFW_KEY_F11] = KeyboardKey::F11;
			glfwToKey[GLFW_KEY_F12] = KeyboardKey::F12;

			glfwToKey[GLFW_KEY_SPACE] = KeyboardKey::Space;
			glfwToKey[GLFW_KEY_ENTER] = KeyboardKey::Enter;
			glfwToKey[GLFW_KEY_ESCAPE] = KeyboardKey::Escape;

			glfwToKey[GLFW_KEY_LEFT]  = KeyboardKey::Left;
			glfwToKey[GLFW_KEY_RIGHT] = KeyboardKey::Right;
			glfwToKey[GLFW_KEY_UP]    = KeyboardKey::Up;
			glfwToKey[GLFW_KEY_DOWN]  = KeyboardKey::Down;

			glfwToKey[GLFW_KEY_KP_ADD]      = KeyboardKey::NumpadAdd;
			glfwToKey[GLFW_KEY_KP_SUBTRACT] = KeyboardKey::NumpadSubtract;
						
			glfwToKey[GLFW_KEY_TAB] = KeyboardKey::Tab;
			glfwToKey[GLFW_KEY_CAPS_LOCK] = KeyboardKey::CapsLock;
			
			glfwToKey[GLFW_KEY_LEFT_SHIFT] = KeyboardKey::LeftShift;
			glfwToKey[GLFW_KEY_RIGHT_SHIFT] = KeyboardKey::RightShift;
			
			glfwToKey[GLFW_KEY_LEFT_CONTROL] = KeyboardKey::LeftCtrl;
			glfwToKey[GLFW_KEY_RIGHT_CONTROL] = KeyboardKey::RightCtrl;
			
			glfwToKey[GLFW_KEY_LEFT_ALT] = KeyboardKey::LeftAlt;
			glfwToKey[GLFW_KEY_RIGHT_ALT] = KeyboardKey::RightAlt;
			
			glfwToKey[GLFW_KEY_LEFT_SUPER] = KeyboardKey::LeftSuper;
			glfwToKey[GLFW_KEY_RIGHT_SUPER] = KeyboardKey::RightSuper;
			
			glfwToKey[GLFW_KEY_BACKSPACE] = KeyboardKey::Backspace;
			glfwToKey[GLFW_KEY_DELETE] = KeyboardKey::Delete;
			glfwToKey[GLFW_KEY_INSERT] = KeyboardKey::Insert;
			
			glfwToKey[GLFW_KEY_HOME] = KeyboardKey::Home;
			glfwToKey[GLFW_KEY_END] = KeyboardKey::End;
			glfwToKey[GLFW_KEY_PAGE_UP] = KeyboardKey::PageUp;
			glfwToKey[GLFW_KEY_PAGE_DOWN] = KeyboardKey::PageDown;
			
			glfwToKey[GLFW_KEY_KP_0] = KeyboardKey::Numpad0;
			glfwToKey[GLFW_KEY_KP_1] = KeyboardKey::Numpad1;
			glfwToKey[GLFW_KEY_KP_2] = KeyboardKey::Numpad2;
			glfwToKey[GLFW_KEY_KP_3] = KeyboardKey::Numpad3;
			glfwToKey[GLFW_KEY_KP_4] = KeyboardKey::Numpad4;
			glfwToKey[GLFW_KEY_KP_5] = KeyboardKey::Numpad5;
			glfwToKey[GLFW_KEY_KP_6] = KeyboardKey::Numpad6;
			glfwToKey[GLFW_KEY_KP_7] = KeyboardKey::Numpad7;
			glfwToKey[GLFW_KEY_KP_8] = KeyboardKey::Numpad8;
			glfwToKey[GLFW_KEY_KP_9] = KeyboardKey::Numpad9;

			glfwToKey[GLFW_KEY_KP_ENTER] = KeyboardKey::NumpadEnter;
			glfwToKey[GLFW_KEY_KP_MULTIPLY] = KeyboardKey::NumpadMultiply;
			glfwToKey[GLFW_KEY_KP_DIVIDE] = KeyboardKey::NumpadDivide;
		}
		private:

		size_t width = 0;
		size_t height = 0;
		std::string	name;
		std::unique_ptr<Context> ctx;
		State state = State::Terminated;
		GLFWwindow* window = nullptr;
		glm::mat4 orthographicProjection;
		MouseEvent mouseEvent;

		static constexpr int KEY_COUNT = GLFW_KEY_LAST + 1;
		std::array<KeyboardKey, KEY_COUNT> glfwToKey;
		std::deque<KeyboardEvent> eventQueue;
	};
}
