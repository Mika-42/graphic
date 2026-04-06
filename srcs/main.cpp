#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <iostream>
#include <memory>

import mka.graphic.window;
import mka.graphic.opengl.renderer;

using namespace mka::graphic;

class MinimalApp : public mka::graphic::Window {
public:
  MinimalApp(std::unique_ptr<mka::graphic::Context> ctx)
      : Window(800, 600, "Minimal", std::move(ctx)) {
		player.geometry = {100, 100, 20, 20};
		player.backgroundColorA = {0.05, 0.05, 0.05, 0.95};
		player.backgroundColorB = {0.05, 0.05, 0.05, 0.95};
		player.radius = {10.0, 10.0, 10.0, 10.0};
		player.shadowColor = {0.0, 0.0, 0.0, 0.5};
		player.shadowSpread = 0.0;
		player.shadowSoftness = 50.0;
	
		bg.geometry = {0, 0, 800, 600};

		bg.backgroundColorA = {0.05, 0.05, 0.05, 0.70};
		bg.backgroundColorB = {0.05, 0.05, 0.05, 0.50};
		bg.texture = gl::loadTexture(IMG_PATH);

		for(size_t i = 0; i < DOTS_COUNT; ++i) {
		dots[i].geometry = {0, 0, 10, 10};
		dots[i].backgroundColorA = {1.0, 1.0, 1.0, 0.3};
		dots[i].backgroundColorB = {1.0, 1.0, 1.0, 0.3};
		dots[i].radius = {5.0, 5.0, 5.0, 5.0};
		dots[i].shadowColor = {1.0, 1.0, 1.0, 0.2};
		dots[i].shadowSpread = 0.0;
		dots[i].shadowSoftness = 100.0;

		dotp[i].x = glm::linearRand(0.0f, 800.f - 100.0f);
		dotp[i].y = glm::linearRand(0.0f, 600.f - 100.0f);
		}
	  }

  void render(const glm::vec2& size, const MouseEventView& /*mouse*/,
              const KeyboardEventView &keyboard, const Time& time) override {

    const glm::mat4 projection = getOrthographicProjection();
    renderer.setBackgroundColor({0.15, 0.15, 0.15, 1});
	
	bg.geometry.z = size.x;
	bg.geometry.w = size.y;

    if(keyboard.isPressed(Key::Up)) {
		player.geometry.y -= speed;
		player.geometry.y = glm::max(player.geometry.y, 0.0f);
	}

    if(keyboard.isPressed(Key::Down)) {
		player.geometry.y += speed;
		player.geometry.y = glm::min(player.geometry.y, size.y - player.geometry.w);
	}

    if(keyboard.isPressed(Key::Left)) {
		player.geometry.x -= speed;
		player.geometry.x = glm::max(player.geometry.x, 0.0f);
	}

    if(keyboard.isPressed(Key::Right)) {
		player.geometry.x += speed;
		player.geometry.x = glm::min(player.geometry.x, size.x - player.geometry.z);
	}


	player.shadowSoftness = 50.0f + 30.0f * glm::abs(glm::cos(glm::pi<float>() * 0.125f * time.now / 1000.0f));

		gl::Rectangle hitbox;
		
		hitbox.geometry = {
			player.geometry.x - player.shadowSoftness + 0.5f * player.geometry.z,
			player.geometry.y - player.shadowSoftness + 0.5f * player.geometry.w,
			2.0f * player.shadowSoftness,
			2.0f * player.shadowSoftness
		};
		
		hitbox.radius = glm::vec4(player.shadowSoftness);

		hitbox.borderColor = {1.0, 1.0, 1.0, 0.05};
		hitbox.borderThickness = 2.0f;


	renderer.add(std::move(bg));
	for(size_t i = 0; i < DOTS_COUNT; ++i) {

		float centerX = dots[i].geometry.x + 0.5 * dots[i].geometry.z;
		float centerY = dots[i].geometry.y + 0.5 * dots[i].geometry.w;
		glm::vec2 center(centerX, centerY);
		dots[i].shadowSoftness = 50.0f + 30.0f * glm::abs(glm::cos(glm::pi<float>() * 0.25f * time.now / 1000.0f + 50.0f * float(i)));
		
		const float dist = gl::distance(hitbox, center);
		if(dist < 0.0f) {
			++score;

			while(gl::distance(hitbox, dotp[i]) < 0.0f) {
			dotp[i].x = glm::linearRand(0.0f, size.x - 50.0f);
			dotp[i].y = glm::linearRand(0.0f, size.y - 50.0f);
			}
		}

		moveDot(dots[i], dotp[i]);
		renderer.add(std::move(dots[i]));
	}

	renderer.add(std::move(player));
	renderer.add(std::move(hitbox));
	renderer.draw(projection);
  }

  void moveDot(gl::Rectangle& dot, const glm::vec2& p) {
	if (dot.geometry.x > p.x) dot.geometry.x -= speed;
	if (dot.geometry.y > p.y) dot.geometry.y -= speed;
	if (dot.geometry.x < p.x) dot.geometry.x += speed;
	if (dot.geometry.y < p.y) dot.geometry.y += speed;
  }

private:
  float speed = 1.0f;
  size_t score = 0;

  gl::Rectangle player;
  gl::Rectangle bg;
 
  static constexpr size_t DOTS_COUNT = 30;
  const char* IMG_PATH = "/home/mika/project/cpp/graphic/example/Lofi-Forest-Wallpaper-4K-Desktop-2.jpg";

  gl::Rectangle dots[DOTS_COUNT];
  glm::vec2 dotp[DOTS_COUNT];
  
  mka::graphic::gl::Renderer<1024> renderer;
};

int main() {
  auto ctx = mka::graphic::createContext(mka::graphic::API::OpenGL,
                                         mka::graphic::Loader::Glad);
  MinimalApp app(std::move(ctx));
  return app.run();
}
