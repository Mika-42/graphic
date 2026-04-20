/**
 * @file sanitize.cppm
 * @brief C++20 module implementation for `sanitize`.
 */
module;
#include <glm/glm.hpp>
#include <cmath>
export module mka.graphic.sanitize;
import mka.graphic.opengl.rectangle;

export namespace mka::graphic {

	/// @brief Sanitize x/y/w/h geometry values and clamp width/height to non-negative.
	void sanitizeGeometry(glm::vec4& g) {
		for (int i = 0; i < 4; i++) {
			if (!std::isfinite(g[i])) {
				g[i] = 0.0f;
			}
		}

		// width / height >= 0
		g.z = glm::max(g.z, 0.0f);
		g.w = glm::max(g.w, 0.0f);
	}

	/// @brief Clamp each corner radius to [0, min(width, height)/2].
	void sanitizeRadius(glm::vec4& radius, const glm::vec2& size) {
		const float maxRadius = glm::min(size.x, size.y) * 0.5f;

		radius = glm::clamp(radius, glm::vec4(0.0f), glm::vec4(maxRadius));
	}
	
	/// @brief Clamp RGBA values to [0,1] and reset non-finite values.
	void sanitizeColor(glm::vec4& color) {
		for (int i = 0; i < 4; i++) {
			if (!std::isfinite(color[i])) {
				color[i] = 0.0f;
			}
		}

		color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
	}
	
	/// @brief Ensure scalar is finite and above a minimum value.
	float sanitizeFloat(float v, float minValue = 0.0f) {
		if (!std::isfinite(v)) return minValue;
		return glm::max(v, minValue);
	}

	/// @brief Keep border thickness non-negative and finite.
	void sanitizeBorderThickness(float& thickness) {
		thickness = sanitizeFloat(thickness, 0.0f);
	}

	/// @brief Sanitize shadow offset/softness/spread parameters.
	void sanitizeShadow(glm::vec2& shadowOffset, float& shadowSoftness, float& shadowSpread) {
		shadowSoftness = sanitizeFloat(shadowSoftness, 0.001f);
		shadowSpread   = sanitizeFloat(shadowSpread, 0.0f);

		for (int i = 0; i < 2; i++) {
			if (!std::isfinite(shadowOffset[i])) {
				shadowOffset[i] = 0.0f;
			}
		}
	}

	/// @brief Clamp/normalize rectangle attributes before uploading to the GPU.
	void sanitizeRectangle(Rectangle &r) {
		sanitizeGeometry(r.geometry);
		sanitizeRadius(r.radius, glm::vec2(r.geometry.z, r.geometry.w));
		sanitizeColor(r.backgroundColorA);
		sanitizeColor(r.backgroundColorB);
		sanitizeColor(r.borderColor);
		sanitizeColor(r.shadowColor);
		sanitizeShadow(r.shadowOffset, r.params.y, r.params.z);
		sanitizeBorderThickness(r.params.w);
		r.params.x = std::fmod(r.params.x, 360.0f);
		if (r.params.x < 0.0f) {
			r.params.x += 360.0f;
		}
	}
}
