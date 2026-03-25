module;
#include "glad.h"
#include <glm/glm.hpp>
#include <vector>

export module mka.graphic.opengl.renderer;
import mka.graphic.opengl.shader;

const auto vs = R"(
				#version 460 core

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
					float _padding0;
					float _padding1;
					float _padding2;
				};

				layout(std430, binding = 0) buffer Rects {
					Rect rects[];
				};

				out vec2 uv;
				out vec4 fillColor;
				out vec4 radius;
				out vec2 rectSize;
				out vec4 borderColor;
				out float borderThickness;
				out vec4 shadowColor;
				out vec2 shadowOffset;
				out float shadowSoftness;
				out float shadowSpread;

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

					uv = aPos;
					fillColor = r.fillColor;
					radius = r.radius;
					rectSize = r.geometry.zw;

					vec2 pos = r.geometry.xy + aPos * r.geometry.zw;

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

				in vec2 uv;
				in vec4 fillColor;
				in vec4 radius;          // xy = top-left, zw = bottom-right
				in vec2 rectSize;
				
				flat in vec4 borderColor;
				flat in float borderThickness;
				flat in vec4 shadowColor;
				flat in vec2 shadowOffset;
				flat in float shadowSoftness;
				flat in float shadowSpread;

				out vec4 FragColor;

				float sdRoundedBox( in vec2 p, in vec2 b, in vec4 r ) {
					r.xy = (p.x>0.0)?r.xy : r.zw;
					r.x  = (p.y>0.0)?r.x  : r.y;
					vec2 q = abs(p)-b+r.x;
					return min(max(q.x,q.y),0.0) + length(max(q,0.0)) - r.x;
				}

				void main() {
					vec2 p = uv * rectSize - 0.5 * rectSize;
					float dist = sdRoundedBox(p, rectSize * 0.5, radius);

					float aa = 0.5;
					float shapeAlpha = smoothstep(aa, -aa, dist); 

					float borderInner = smoothstep(aa, -aa, dist + borderThickness);
				    float borderOuter = shapeAlpha;
					float borderMask = borderOuter - borderInner;

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

					vec4 shapeColor = mix(fillColor, borderColor, borderMask);
					float shapeAlphaCombined = max(shapeAlpha, borderMask);
					vec4 composedShape = vec4(shapeColor.rgb, shapeColor.a * shapeAlphaCombined);

					// Draw shadow only where the shape is not fully opaque to keep it behind.
					float visibleShadow = shadowAlpha * (1.0 - shapeAlphaCombined);
					vec4 composedShadow = vec4(shadowColor.rgb, shadowColor.a * visibleShadow);

					FragColor = composedShape + composedShadow;
				}
			)";

//OpenGL
namespace mka::graphic::gl {
	
	export struct alignas(16) Rectangle {
		glm::vec4 geometry {}; //x, y, w, h
		glm::vec4 radius {};
		glm::vec4 fillColor {};
		glm::vec4 borderColor {};
		glm::vec4 shadowColor {0.0f, 0.0f, 0.0f, 0.35f};
		glm::vec2 shadowOffset {0.0f, 10.0f};
		float shadowSoftness {24.0f};
		float shadowSpread {0.0f};
		float borderThickness {};
		float _padding0 {};
		float _padding1 {};
		float _padding2 {};
	};

	void sanitizeRadius(glm::vec4& radius, const glm::vec2& size) {
		const float maxRadius = glm::min(size.x, size.y) * 0.5f;
		radius.x = glm::min(radius.x, maxRadius);
		radius.y = glm::min(radius.y, maxRadius);
		radius.z = glm::min(radius.z, maxRadius);
		radius.w = glm::min(radius.w, maxRadius);
	}

	void sanitizeBorderThickness(float& thickness) {
		thickness = glm::max(thickness, 0.0f);
	}

	void sanitizeShadow(glm::vec4& shadowColor, float& shadowSoftness) {
		shadowColor.a = glm::clamp(shadowColor.a, 0.0f, 1.0f);
		// Keep the transition stable and avoid division by zero equivalent cases in smoothstep.
		shadowSoftness = glm::max(shadowSoftness, 0.001f);
	}

	export class Renderer {
		
		public:

			Renderer() {
				rectangles.reserve(capacity);
				shader.addScript(vs, ShaderType::Vertex);
				shader.addScript(fs, ShaderType::Fragment);
				shader.link();
				
				// empty vao (without it doesn't work...)
				glCreateVertexArrays(1, &vao);

				// create an ssbo
				glCreateBuffers(1, &ssbo);
				glNamedBufferData(
						ssbo, 
						capacity * sizeof(Rectangle), 
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

			[[maybe_unused]] Rectangle& add(Rectangle r) {
				rectangles.emplace_back(std::move(r));
				return rectangles.back();
			}

			void draw(const glm::mat4 projection) {
				if (rectangles.empty()) return;
				
				for (auto& r : rectangles) {
					sanitizeRadius(r.radius, glm::vec2(r.geometry.z, r.geometry.w));
					sanitizeBorderThickness(r.borderThickness);
					sanitizeShadow(r.shadowColor, r.shadowSoftness);
				}

				glNamedBufferSubData(
					ssbo,
					0,
					rectangles.size() * sizeof(Rectangle),
					rectangles.data()
				);
				
				// bind ssbo
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

				// Draw
				shader.use();
				shader.set("uProjection", projection);
				
				glBindVertexArray(vao);

				glDrawArraysInstanced(GL_TRIANGLES, 0, 6, rectangles.size());
			}

		private:
			std::vector<Rectangle> rectangles;
			Shader shader;

			GLuint vao = 0;
			GLuint ssbo = 0;
			size_t capacity = 1024;
	};
}
