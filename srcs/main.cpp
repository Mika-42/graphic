
#include "glad.h"
#include <print>
#include <memory>
#include <glm/glm.hpp>

import mka.graphic.window;
import mka.graphic.opengl.renderer;

class MyApp : public mka::graphic::Window {
	public:	

		MyApp(std::unique_ptr<mka::graphic::Context> ctx) :
			mka::graphic::Window(800, 200, "Hello Window", std::move(ctx)) {
		
		renderer.add({
			.geometry = {0, 200, 200, 200},
			.radius = {0.0f, 0.0f, 0.0f, 0.0f},
			.fillColor = {0.0f, 1.0f, 0.0f, 1.0f},
			.borderColor = {0.0f, 0.0f, 0.0f, 0.0f},
			.borderThickness = {0.0f, 0.0f, 0.0f, 0.0f},
		});	
	}

	void render() {
		glClearColor(1.0f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);	
		
		glm::mat4 projection = getOrthographicProjection();
		renderer.draw(projection);
	}

	private:
		mka::graphic::gl::Renderer renderer;
};

int main() {

	auto ctx = mka::graphic::createContext(
			mka::graphic::API::OpenGL, 
			mka::graphic::Loader::Glad
	);
	MyApp w(std::move(ctx));
			
	w.run();

	return 0;
}
