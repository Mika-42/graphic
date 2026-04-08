module;

#include "glad.h"
#include <glm/glm.hpp>
#include "debug.hpp"
#include "stb_image.h"
export module mka.graphic.opengl.rectangle;

//OpenGL
export namespace mka::graphic {
	
	/**
	 * @brief GPU instance payload for rectangle/text rendering.
	 *
	 * Memory layout is mirrored in GLSL `Rect` struct inside the SSBO.
	 */
	struct alignas(16) Rectangle {
		glm::vec4 geometry {}; //x, y, w, h
		glm::vec4 radius {};
		glm::vec4 backgroundColorA {};
		glm::vec4 backgroundColorB {};
		glm::vec4 borderColor {};
		glm::vec4 shadowColor {};
		glm::vec2 shadowOffset {};
		float gradientAngle {};
		float shadowSoftness {};
		float shadowSpread {};
		float borderThickness {};
		uint64_t texture {};
		uint32_t flags {};
		uint32_t _pad {};
	};

	float distance(const Rectangle& rect, glm::vec2 point) {
		glm::vec2 pos(rect.geometry.x, rect.geometry.y);
		glm::vec2 size(rect.geometry.z, rect.geometry.w);

		glm::vec2 center = pos + size * 0.5f;
		glm::vec2 p = point - center;
		glm::vec2 b = size * 0.5f;

		glm::vec4 r = rect.radius;

		r.x = (p.x > 0.0f) ? r.x : r.z;
		r.y = (p.x > 0.0f) ? r.y : r.w;
		r.x = (p.y > 0.0f) ? r.x : r.y;

		glm::vec2 q = glm::abs(p) - b + r.x;

		return std::min(std::max(q.x, q.y), 0.0f) + glm::length(glm::max(q, glm::vec2(0.0f))) - r.x;
	}

	/// @brief Load an RGBA texture and return its bindless texture handle.
	uint64_t loadTexture(const char* path) {
		if (path == nullptr) {
			DEBUG_LOG("loadTexture called with null path pointer.");
			return 0;
		}

		int width, height, channels;
		stbi_set_flip_vertically_on_load(false);

		unsigned char* data = stbi_load(path, &width, &height, &channels, 4);
		if (!data) {
			DEBUG_LOG("Failed to load texture: " + std::string(path));
			return 0;
		}
		if (width <= 0 || height <= 0) {
			DEBUG_LOG("Invalid texture dimensions for: " + std::string(path));
			stbi_image_free(data);
			return 0;
		}

		GLuint tex = 0;
		glCreateTextures(GL_TEXTURE_2D, 1, &tex);
		if (tex == 0) {
			DEBUG_LOG("glCreateTextures returned 0 for: " + std::string(path));
			stbi_image_free(data);
			return 0;
		}

		glTextureStorage2D(tex, 1, GL_RGBA8, width, height);

		glTextureSubImage2D(
			tex,
			0,
			0, 0,
			width, height,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			data
		);

		glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		stbi_image_free(data);

		uint64_t handle = glGetTextureHandleARB(tex);
		if (handle == 0) {
			DEBUG_LOG("glGetTextureHandleARB returned null handle for: " + std::string(path));
			glDeleteTextures(1, &tex);
			return 0;
		}
		glMakeTextureHandleResidentARB(handle);

		DEBUG_LOG("Texture loaded: " + std::string(path) + " (" + std::to_string(width) + "x" + std::to_string(height) + ")");
		return handle;
	}
}
