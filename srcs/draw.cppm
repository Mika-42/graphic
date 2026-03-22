module;

#include <glm/glm.hpp>
#include <vector>

export module mka.graphic.draw;

//OpenGL
export namespace mka::graphic {
	
	struct Rectangle {
		glm::vec2 pos {};
		glm::vec2 size {};
		glm::vec4 radius {};
		glm::vec4 fillColor {};
		glm::vec4 borderColor {};
		float borderThickness = 0.0f;
	};

	class Renderer {
		
		public:

			Render() {
				render.reserve(1024);
			}

			void add(Rectangle r) {
				rectangles.emplace_back(std::move(r));
			}

			void draw() {
				//todo
			}

		private:
			std::vector<Rectangle> rectangles;
	}
}
