#include <memory>
#include <glm/glm.hpp>

import mka.graphic.window;
import mka.graphic.opengl.renderer;

class MinimalApp : public mka::graphic::Window {
public:
    MinimalApp(std::unique_ptr<mka::graphic::Context> ctx)
        : Window(800, 600, "Minimal", std::move(ctx)) {}

    void render() override {

		static float t = 0.0f;
		t += 0.01f;

        const glm::mat4 projection = getOrthographicProjection();
        renderer.setBackgroundColor({0.5, 0.5, 0.5, 1});
        renderer.add({
            .geometry = {80, 80, 240, 120},
			.radius = glm::vec4(20.0f),
            .backgroundColorA = {0.2f, 0.4f, 0.8f, 1.0f},
            .backgroundColorB = {0.1f, 0.8f, 0.7f, 1.0f},
			.borderColor = {1.0f, 1.0f, 1.0f, 1.0f},
			.shadowColor = {0.0f, 0.0f, 0.0f, 0.5f},
			.shadowOffset = {5.0f, 5.0f},
            .gradientAngle = 20.0f,
			.shadowSoftness = 50.0f,
			.shadowSpread = 5.0f,
			.borderThickness = 5.0f
        });

		renderer.add({
			.content = "Minimal",
			.font = "/home/mika/Downloads/Winter Draw.ttf",
			.color = {1.0, 0.0, 0.0, 1.0},
			.gradientColorA = {1.0, 0.0, 0.0, 1.0},
			.gradientColorB = {0.0, 0.0, 1.0, 1.0},
			.gradientAngle = 360.0f * glm::cos(t),
			.position = { 100, 100 },
			.fontSize = 40,
			.letterSpacing = 3		
		});

        renderer.draw(projection);
    }

private:
    mka::graphic::gl::Renderer<1024> renderer;
};

int main() {
    auto ctx = mka::graphic::createContext(
        mka::graphic::API::OpenGL,
        mka::graphic::Loader::Glad
    );
    MinimalApp app(std::move(ctx));
    return app.run();
}
