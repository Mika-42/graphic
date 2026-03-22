module;
#include "glad.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
export module mka.graphic.context;

export namespace mka::graphic {

	enum class API {
		None, OpenGL, Vulkan, DirectX, Metal
	};

	enum class Loader {
		None, Glad, Glew
	};

	class Context {
		public:
			virtual bool init(GLFWwindow* window) = 0;
			virtual std::string getName() const = 0;	
			virtual API getAPI() const = 0;
			virtual void makeCurrent() {}
			virtual void swapBuffers() {}
			virtual ~Context() {}
		protected:
			GLFWwindow* window = nullptr;
	};
}

namespace mka::graphic {

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

	// Not implemented [todo]
	class GlewContext final: public Context {
		public:
			bool init(GLFWwindow* /*window*/) override {
				return false; //todo  
			}
			
			std::string getName() const override { return "OpenGL Glew"; }
			API getAPI() const override { return API::OpenGL; }
	}; 
	class VulkanContext final: public Context {
		public:
			bool init(GLFWwindow* /*window*/) override {
				return false; //todo  
			}
			
			std::string getName() const override { return "Vulkan"; }
			API getAPI() const override { return API::Vulkan; }
	}; 
	class DirectXContext final: public Context {
		public:
			bool init(GLFWwindow* /*window*/) override {
				return false; //todo  
			}
			
			std::string getName() const override { return "DirectX"; }
			API getAPI() const override { return API::DirectX; }
	}; 
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

	std::unique_ptr<Context> createContext(API api, Loader loader = Loader::None) {
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
