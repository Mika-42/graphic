
#include "glad.h"
#include <print>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

import mka.graphic.window;
import mka.graphic.opengl.renderer;

float map(float x, float a, float b, float c, float d)
{
    return c + (x - a) * (d - c) / (b - a);
}

class MyApp : public mka::graphic::Window {
	public:	

		MyApp(std::unique_ptr<mka::graphic::Context> ctx) :
			mka::graphic::Window(800, 200, "Function Plotter", std::move(ctx)) {
			renderer.setBackgroundColor({1.0f});	
		}

	void render() {
			
		const glm::mat4 projection = getOrthographicProjection();
	
		// x axis
		renderer.add({
            .geometry = {0, getSize().y / 2, getSize().x, 2},
            .fillColor = black,
        });
		
		// y axis
		renderer.add({
			.geometry = {getSize().x / 2, 0, 2, getSize().y},
			.fillColor = black,
		});

		auto drawFn = [&](float x, float y, glm::vec4 color) {	
			renderer.add({
				.geometry = {
					map(x, -(getSize().x / 2), (getSize().x / 2),0.0f, getSize().x),
					map(0.5 * y, -1.0, 1.0, 0.0f, getSize().y),
					5, 5},
				.radius = glm::vec4(2.5f),
				.fillColor = color,
			});
		};

		for(int i = -(getSize().x / 2); i < (getSize().x / 2); ++i) {
			drawFn(i, glm::cos(2.0f * 3.1415f * 44.0f * i), red);
		}
		for(int i = -(getSize().x / 2); i < (getSize().x / 2); ++i) {
			drawFn(i, glm::sin(2.0f * 3.1415f * 88.0f * i), blue);
		}
		for(int i = -(getSize().x / 2); i < (getSize().x / 2); ++i) {
			drawFn(i, glm::sin(2.0f * 3.1415f * 110.0f * i), green);
		}

		renderer.draw(projection);	
	}

	private:
		mka::graphic::gl::Renderer<4096> renderer;
		const glm::vec4 black	{0.0f, 0.0f, 0.0f, 1.0f};
		const glm::vec4 red		{1.0f, 0.0f, 0.0f, 1.0f};
		const glm::vec4 blue	{0.0f, 0.0f, 1.0f, 1.0f};
		const glm::vec4 green	{0.0f, 1.0f, 0.0f, 1.0f};
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

