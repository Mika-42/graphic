module;
#include <string>
#include <glm/glm.hpp>
export module.graphic.opengl.text;

export namespace mka::graphic {

	struct Text {
		std::string content {};
		std::string font {};
		glm::vec4 color {};
		glm::vec4 gradientColorA {};
		glm::vec4 gradientColorB {};
		float gradientAngle {};
		glm::vec2 position {};
		float fontSize {};
		float letterSpacing {};
	};
} // namespace mka::graphic
