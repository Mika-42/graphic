
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
			renderer.setBackgroundColor(glm::vec4{1.0f});
		}

	void render() {
		static double t = 0.0;	
		const glm::mat4 projection = getOrthographicProjection();
	
		// x axis
		renderer.add({
            .geometry = {70, 40, 400, 200},
            .radius = {40, 100, 20, 50},
			.fillColor = green,
			.borderColor = red,
			.shadowColor = blue,
			.shadowSoftness = glm::abs(glm::cos(float(t * 10.0))) * 100.0f, 
			.borderThickness = 5.0f,
			.texture = mka::graphic::gl::loadTexture("/home/mika/Downloads/welcome-totoro.jpg")
        });
		
		renderer.draw(projection);
		t += 0.01;
	}

	private:
		mka::graphic::gl::Renderer<4096> renderer;
		const glm::vec4 black	{0.0f, 0.0f, 0.0f, 1.0f};
		const glm::vec4 red		{1.0f, 0.0f, 0.0f, 1.0f};
		const glm::vec4 blue	{0.0f, 0.0f, 1.0f, 1.0f};
		const glm::vec4 green	{0.0f, 1.0f, 0.0f, 0.5f};
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
