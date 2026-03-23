module;

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string>

export module mka.graphic.window;
export import mka.graphic.context;


void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height) {
    glViewport(0, 0, width, height);
}

export namespace mka::graphic {
	

	enum class State {
		Inited, Running, Stopped, Terminated
	};

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
				});
				
				if(!this->ctx || !this->ctx->init(window)) {
					state = State::Terminated;
					return;
				}
			}
			
			virtual void render() = 0;

			const glm::mat4& getOrthographicProjection() const {
				return orthographicProjection;
			}

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

		private:

		size_t width = 0;
		size_t height = 0;
		std::string	name;
		std::unique_ptr<Context> ctx;
		State state = State::Terminated;
		GLFWwindow* window = nullptr;
		glm::mat4 orthographicProjection;
	};
}
