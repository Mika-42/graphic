module;
#include "glad.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
export module mka.graphic.context;

export namespace mka::graphic {

	/**
	 * @brief Rendering backend abstraction used by the windowing layer.
	 */
	enum class API {
		None, OpenGL, Vulkan, DirectX, Metal
	};

	/**
	 * @brief OpenGL function loader selection.
	 */
	enum class Loader {
		None, Glad, Glew
	};

	/**
	 * @brief Base interface for a graphics context implementation.
	 *
	 * The interface is intentionally minimal so each backend can manage
	 * initialization and per-frame buffer swapping in its own way.
	 */
	class Context {
		public:
			/// Initialize backend state for a given native window.
			virtual bool init(GLFWwindow* window) = 0;
			/// Return a human-readable backend name.
			virtual std::string getName() const = 0;	
			/// Return the backend API kind.
			virtual API getAPI() const = 0;
			/// Make this context current on the calling thread.
			virtual void makeCurrent() {}
			/// Present the current frame.
			virtual void swapBuffers() {}
			virtual ~Context() {}
		protected:
			GLFWwindow* window = nullptr;
	};
}

namespace mka::graphic {

	/**
	 * @brief OpenGL context implementation using GLAD.
	 */
	class GladContext final: public Context {
		public:

		bool init(GLFWwindow* window) override {
			if (!window) return false;
						
			this->window = window;
			makeCurrent();

			if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
				return false;
			}

			return true;
		}

		std::string getName() const override { return "OpenGL Glad"; }
		API getAPI() const override { return API::OpenGL; }

		void makeCurrent() override {	
			if(window) glfwMakeContextCurrent(window);
		}

		void swapBuffers() override {
			if(window) glfwSwapBuffers(window);
		}

	};

	/// @brief Placeholder OpenGL context implementation for GLEW.
	class GlewContext final: public Context {
		public:
			bool init(GLFWwindow* /*window*/) override {
				return false; //todo  
			}
			
			std::string getName() const override { return "OpenGL Glew"; }
			API getAPI() const override { return API::OpenGL; }
	}; 
	/// @brief Placeholder Vulkan context implementation.
	class VulkanContext final: public Context {
		public:
			bool init(GLFWwindow* /*window*/) override {
				return false; //todo  
			}
			
			std::string getName() const override { return "Vulkan"; }
			API getAPI() const override { return API::Vulkan; }
	}; 
	/// @brief Placeholder DirectX context implementation.
	class DirectXContext final: public Context {
		public:
			bool init(GLFWwindow* /*window*/) override {
				return false; //todo  
			}
			
			std::string getName() const override { return "DirectX"; }
			API getAPI() const override { return API::DirectX; }
	}; 
	/// @brief Placeholder Metal context implementation.
	class MetalContext final: public Context {
		public:
			bool init(GLFWwindow* /*window*/) override {
				return false; //todo  
			}
			
			std::string getName() const override { return "Metal"; }
			API getAPI() const override { return API::Metal; }
	}; 
}

export namespace mka::graphic {

	/**
	 * @brief Create a concrete context from a backend API + loader pair.
	 */
		[[nodiscard]] std::unique_ptr<Context> createContext(API api, Loader loader = Loader::None) {
		switch(api) {
			case API::Vulkan: return std::make_unique<VulkanContext>();
			case API::DirectX: return std::make_unique<DirectXContext>();
			case API::Metal: return std::make_unique<MetalContext>();
			case API::OpenGL:
				switch(loader) {
					case Loader::Glad: return std::make_unique<GladContext>();
					case Loader::Glew: return std::make_unique<GlewContext>();
					case Loader::None: return nullptr;
					default: return nullptr;
				}
			case API::None: return nullptr;
			default: return nullptr;
		}
	}
} // mka::graphic
