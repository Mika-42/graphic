module;
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "glad.h"
#include <glm/glm.hpp>
#include <array>
#include <iostream>

export module mka.graphic.opengl.renderer;
import mka.graphic.opengl.shader;

const auto vs = R"(
				#version 460 core
				#extension GL_ARB_bindless_texture : require

				struct Rect {
					vec4 geometry; // xy = pos, zw = size
					vec4 radius;
					vec4 fillColor;
					vec4 borderColor;
					vec4 shadowColor;
					vec2 shadowOffset;
					float shadowSoftness;
					float shadowSpread;
					float borderThickness;
					uvec2 texture;
					float _padding0;
					float _padding1;
				};

				layout(std430, binding = 0) buffer Rects {
					Rect rects[];
				};

				out vec2 localPoint;
				out vec4 fillColor;
				out vec4 radius;
				out vec2 rectSize;
				out vec4 borderColor;
				out float borderThickness;
				out vec4 shadowColor;
				out vec2 shadowOffset;
				out float shadowSoftness;
				out float shadowSpread;
				out vec2 texCoord;
				flat out uvec2 textureHandle;

				uniform mat4 uProjection;
				
				void main() {

					vec2 quad[6] = vec2[](
						vec2(0.0, 0.0),
						vec2(1.0, 0.0),
						vec2(1.0, 1.0),

						vec2(0.0, 0.0),
						vec2(1.0, 1.0),
						vec2(0.0, 1.0)
					);

					vec2 aPos = quad[gl_VertexID];
					Rect r = rects[gl_InstanceID];
					texCoord = aPos;
					textureHandle = r.texture;

					fillColor = r.fillColor;
					radius = r.radius;
					rectSize = r.geometry.zw;

					// Expand the instance quad so the shadow can be drawn outside the rectangle bounds.
					float shadowExtent = max(r.shadowSoftness, 0.0) + max(r.shadowSpread, 0.0);
					vec2 pad = abs(r.shadowOffset) + vec2(shadowExtent + 1.0);
					vec2 expandedSize = r.geometry.zw + 2.0 * pad;
					vec2 expandedPos = r.geometry.xy - pad;

					vec2 pos = expandedPos + aPos * expandedSize;
					localPoint = (-pad + aPos * expandedSize) - (0.5 * r.geometry.zw);

					gl_Position = uProjection * vec4(pos, 0.0, 1.0);
					borderColor = r.borderColor;
					borderThickness = r.borderThickness;
					shadowColor = r.shadowColor;
					shadowOffset = r.shadowOffset;
					shadowSoftness = r.shadowSoftness;
					shadowSpread = r.shadowSpread;
				}
				)";

const auto fs = R"(
				#version 460 core
				#extension GL_ARB_bindless_texture : require

				in vec2 localPoint;
				in vec4 fillColor;
				in vec4 radius;          // xy = top-left, zw = bottom-right
				in vec2 rectSize;
				in vec2 texCoord;
				
				in vec4 borderColor;
				in float borderThickness;
				in vec4 shadowColor;
				in vec2 shadowOffset;
				in float shadowSoftness;
				in float shadowSpread;
				flat in uvec2 textureHandle;

				out vec4 FragColor;

				float sdRoundedBox( in vec2 p, in vec2 b, in vec4 r ) {
					r.xy = (p.x>0.0)?r.xy : r.zw;
					r.x  = (p.y>0.0)?r.x  : r.y;
					vec2 q = abs(p)-b+r.x;
					return min(max(q.x,q.y),0.0) + length(max(q,0.0)) - r.x;
				}

				void main() {
					vec2 p = localPoint;
					float dist = sdRoundedBox(p, rectSize * 0.5, radius);

					float aa = 0.5;
					float shapeAlpha = smoothstep(aa, -aa, dist); 

					float borderInner = smoothstep(aa, -aa, dist + borderThickness);
				    float borderOuter = shapeAlpha;
					float borderMask = clamp(borderOuter - borderInner, 0.0, 1.0);

					// Signed distance of the shadow comes from the same rounded box SDF,
					// but translated and expanded to produce a soft halo behind the shape.
					vec2 shadowPoint = p - shadowOffset;
					float shadowDist = sdRoundedBox(
						shadowPoint,
						(rectSize * 0.5) + vec2(shadowSpread),
						radius
					);

					// Softness controls the falloff width around the shadow edge.
					float safeShadowSoftness = max(shadowSoftness, 0.001);
					float shadowAlpha = 1.0 - smoothstep(-safeShadowSoftness, safeShadowSoftness, shadowDist);
					shadowAlpha = clamp(shadowAlpha, 0.0, 1.0);

					// Use bindless texture only when a non-null GPU handle is available.
					// This keeps untextured rectangles on the fast path.
					vec4 baseFill = fillColor;
					if (any(notEqual(textureHandle, uvec2(0u)))) {
						sampler2D rectTexture = sampler2D(textureHandle);
						baseFill = mix(texture(rectTexture, texCoord), fillColor, fillColor.a);
					}

					vec4 shapeColor = mix(baseFill, borderColor, borderMask);
					float shapeCoverage = max(shapeAlpha, borderMask);
					float shapeAlphaCombined = clamp(shapeColor.a * shapeCoverage, 0.0, 1.0);

					// Keep the shadow behind the visible shape and composite in straight alpha.
					float shadowCombinedAlpha = clamp(shadowColor.a * shadowAlpha, 0.0, 1.0);
					float visibleShadow = shadowCombinedAlpha * (1.0 - shapeAlphaCombined);
					float outAlpha = shapeAlphaCombined + visibleShadow;

					vec3 premulRgb = (shapeColor.rgb * shapeAlphaCombined) + (shadowColor.rgb * visibleShadow);
					vec3 outRgb = (outAlpha > 0.0) ? (premulRgb / outAlpha) : vec3(0.0);
					FragColor = vec4(outRgb, outAlpha);
				}
			)";

//OpenGL
namespace mka::graphic::gl {
	
	export struct alignas(16) Rectangle {
		glm::vec4 geometry {}; //x, y, w, h
		glm::vec4 radius {};
		glm::vec4 fillColor {};
		glm::vec4 borderColor {};
		glm::vec4 shadowColor {};
		glm::vec2 shadowOffset {0.0f, 10.0f};
		float shadowSoftness {24.0f};
		float shadowSpread {};
		float borderThickness {};
		uint64_t texture {};
		float _padding0 {};
		float _padding1 {};
	};
	
	export uint64_t loadTexture(const char* path) {
		int width, height, channels;
		stbi_set_flip_vertically_on_load(false);

		unsigned char* data = stbi_load(path, &width, &height, &channels, 4);
		if (!data) {
			return 0;
		}

		GLuint tex = 0;
		glCreateTextures(GL_TEXTURE_2D, 1, &tex);

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
		glMakeTextureHandleResidentARB(handle);

		return handle;
	}

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

	void sanitizeRadius(glm::vec4& radius, const glm::vec2& size) {
		const float maxRadius = glm::min(size.x, size.y) * 0.5f;

		radius = glm::clamp(radius, glm::vec4(0.0f), glm::vec4(maxRadius));
	}
	
	void sanitizeColor(glm::vec4& color) {
		for (int i = 0; i < 4; i++) {
			if (!std::isfinite(color[i])) {
				color[i] = 0.0f;
			}
		}

		color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
	}
	
	float sanitizeFloat(float v, float minValue = 0.0f) {
		if (!std::isfinite(v)) return minValue;
		return glm::max(v, minValue);
	}

	void sanitizeBorderThickness(float& thickness) {
		thickness = sanitizeFloat(thickness, 0.0f);
	}

	void sanitizeShadow(glm::vec2& shadowOffset, float& shadowSoftness, float& shadowSpread) {
		shadowSoftness = sanitizeFloat(shadowSoftness, 0.001f);
		shadowSpread   = sanitizeFloat(shadowSpread, 0.0f);

		for (int i = 0; i < 2; i++) {
			if (!std::isfinite(shadowOffset[i])) {
				shadowOffset[i] = 0.0f;
			}
		}
	}

	void sanitizeRectangle(Rectangle &r) {
		sanitizeGeometry(r.geometry);
		sanitizeRadius(r.radius, glm::vec2(r.geometry.z, r.geometry.w));
		sanitizeColor(r.fillColor);
		sanitizeColor(r.borderColor);
		sanitizeColor(r.shadowColor);
		sanitizeShadow(r.shadowOffset, r.shadowSoftness, r.shadowSpread);
		sanitizeBorderThickness(r.borderThickness);
	}

	export template<size_t MAX_RECTANGLE_COUNT> class Renderer {
		
		public:

			Renderer() {
				std::cerr << shader.addScript(vs, ShaderType::Vertex) << '\n'; 
				std::cerr << shader.addScript(fs, ShaderType::Fragment) << '\n'; 
				shader.link();
				
				// empty vao (without it doesn't work...)
				glCreateVertexArrays(1, &vao);

				// create an ssbo
				glCreateBuffers(1, &ssbo);
				glNamedBufferData(
						ssbo, 
						MAX_RECTANGLE_COUNT * sizeof(Rectangle), 
						nullptr,
						GL_DYNAMIC_DRAW
				);
			}
			
			~Renderer() {
				if (ssbo != 0) {
					glDeleteBuffers(1, &ssbo);
				}
				if (vao != 0) {
					glDeleteVertexArrays(1, &vao);
				}
			}

			[[maybe_unused]] Rectangle* add(Rectangle&& r) {
				if (rectangleCount >= MAX_RECTANGLE_COUNT) {
					return nullptr; // ou assert
				}
				rectangles[rectangleCount] = r;
				return &rectangles[rectangleCount++];
			}

			void draw(const glm::mat4 projection) {
			
				glClearColor(bgColor.r, bgColor.g, bgColor.b, bgColor.a);
				glClear(GL_COLOR_BUFFER_BIT);

				if (rectangleCount == 0) return;
				
				for (size_t i = 0; i < rectangleCount; ++i) {
					sanitizeRectangle(rectangles[i]);
				}

				glNamedBufferSubData(
					ssbo,
					0,
					rectangleCount * sizeof(Rectangle),
					rectangles.data()
				);
				
				// bind ssbo
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

				// Draw
				shader.use();
				shader.set("uProjection", projection);
				
				glBindVertexArray(vao);

				glDrawArraysInstanced(GL_TRIANGLES, 0, 6, rectangleCount);

				rectangleCount = 0;
			}

			void setBackgroundColor(const glm::vec4& color) {
				bgColor = color;	
				sanitizeColor(bgColor);
			}

		private:
			std::array<Rectangle, MAX_RECTANGLE_COUNT> rectangles;
			size_t rectangleCount = 0;

			glm::vec4 bgColor = glm::vec4{1.0f};
			Shader shader;

			GLuint vao = 0;
			GLuint ssbo = 0;
	};
}
