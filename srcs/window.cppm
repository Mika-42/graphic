module;

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string>

export module mka.graphic.window;
export import mka.graphic.context;

/// @brief Keep OpenGL viewport in sync with framebuffer size.
void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height) {
    glViewport(0, 0, width, height);
}

export namespace mka::graphic {
	
	/// @brief High-level runtime state of a window instance.
	enum class State {
		Inited, Running, Stopped, Terminated
	};

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
			Window(size_t width, size_t height, const std::string& name, std::unique_ptr<Context> ctx) 
				: width(width), height(height), name(name), ctx(std::move(ctx)), window(nullptr) {
				if(!glfwInit()) {
					state = State::Terminated;
					return;
				}

				state = State::Inited;

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
				});
				glfwSetCursorPosCallback(window,
					[](GLFWwindow* win, double x, double y) {	
						Window* self = static_cast<Window*>(glfwGetWindowUserPointer(win));
						self->mousePos.x = x;
						self->mousePos.y = y;
				});

				if(!this->ctx || !this->ctx->init(window)) {
					state = State::Terminated;
					return;
				}
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			
			virtual void render() = 0;

			/// @brief Projection updated whenever the window size changes.
			const glm::mat4& getOrthographicProjection() const {
				return orthographicProjection;
			}

			/// @brief Run the blocking frame loop until close.
			int run() {
				if(state != State::Inited) return -1;

				state = State::Running;

				while(!glfwWindowShouldClose(window)) {
					
					ctx->makeCurrent();

					render();

					ctx->swapBuffers();

					glfwPollEvents();
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

			const glm::vec2& getMousePos() {
				return mousePos;
			}

		private:

		size_t width = 0;
		size_t height = 0;
		std::string	name;
		std::unique_ptr<Context> ctx;
		State state = State::Terminated;
		GLFWwindow* window = nullptr;
		glm::mat4 orthographicProjection;
		glm::vec2 mousePos;
	};
}
