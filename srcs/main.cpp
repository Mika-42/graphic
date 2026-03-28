
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
	
		const glm::mat4 projection = getOrthographicProjection();
	
		// x axis
/*		renderer.add({
            .geometry = {70, 40, 400, 200},
            .radius = {40, 100, 20, 50},
			.backgroundColor = green,
			.borderColor = red,
			.shadowColor = blue,
			.borderThickness = 5.0f,
			.texture = mka::graphic::gl::loadTexture("/home/mika/Downloads/welcome-totoro.jpg")
        });
*/
		renderer.add({
				.content = "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
				.font = "/home/mika/Downloads/Winter Draw.ttf",
				.color = black,
				.position = {10, 50},
				.fontSize = 50,
		});


		renderer.add({
				.content = "abcdefghijklmnopqrstuvwxyz",
				.font = "/home/mika/Downloads/Winter Draw.ttf",
				.color = black,
				.position = {10, 150},
				.fontSize = 50,
		});

		renderer.add({
				.content = "0123456789.,;:@#'!\"/?<>%&*()$ ",
				.font = "/home/mika/Downloads/Winter Draw.ttf",
				.color = black,
				.position = {10, 250},
				.fontSize = 50,
		});
		renderer.draw(projection);	
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
