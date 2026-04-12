module;
#include <string>
#include <cmath>
#include <glm/glm.hpp>

export module mka.graphic.opengl.text;
import mka.graphic.sanitize;

export namespace mka::graphic {

	struct Text {
		std::string content {};
		std::string font {};
		glm::vec4 gradientColorA {};
		glm::vec4 gradientColorB {};
		// false: solid text color from gradientColorA
		// true: directional gradient from gradientColorA -> gradientColorB
		bool useGradient {false};
		float gradientAngle {};
		glm::vec2 position {};
		float fontSize {};
		float letterSpacing {};
	};

	/// @brief Sanitize text command values prior to glyph generation.
	void sanitizeText(Text &t) {
		sanitizeColor(t.gradientColorA);
		sanitizeColor(t.gradientColorB);
		t.gradientAngle = std::fmod(t.gradientAngle, 360.0f);
		if (t.gradientAngle < 0.0f) {
			t.gradientAngle += 360.0f;
		}
		t.fontSize = sanitizeFloat(t.fontSize, 0.0f);
		t.letterSpacing = sanitizeFloat(t.letterSpacing, 0.0f);

		if (!std::isfinite(t.position.x)) t.position.x = 0.0f;
		if (!std::isfinite(t.position.y)) t.position.y = 0.0f;
	}
} // namespace mka::graphic
