
#include "glad.h"
#include <print>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

import mka.graphic.window;
import mka.graphic.opengl.renderer;

class MyApp : public mka::graphic::Window {
	public:	

		MyApp(std::unique_ptr<mka::graphic::Context> ctx) :
			mka::graphic::Window(800, 200, "Hello Window", std::move(ctx)) {
		
		renderer.add({
			.geometry = {50, 20, 200, 200},
			.radius = {10.0f, 50.0f, 100.0f, 200.0f},
			.fillColor = {0.0f, 1.0f, 0.0f, 1.0f},
			.borderColor = {1.0f, 1.0f, 1.0f, 1.0f},
			.shadowColor = {1.0f, 0.0f, 0.0f, 1.0f},
			.shadowOffset = {0.0f, 0.0f},
			.shadowSoftness = 50.0f,
			.shadowSpread = 10.0f,
			.borderThickness = 15.0f,

		});	
	}

	void render() {
		glClearColor(0.0f, 0.1f, 0.1f, 1.0f);
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
