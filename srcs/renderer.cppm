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
					float borderThickness;
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

					vec4 color = mix(fillColor, borderColor, borderMask);
					float alpha = max(shapeAlpha, borderMask);

					FragColor = vec4(color.rgb, color.a * alpha);				
				}
			)";

//OpenGL
namespace mka::graphic::gl {
	
	export struct Rectangle {		
		glm::vec4 geometry {}; //x, y, w, h
		glm::vec4 radius {};
		glm::vec4 fillColor {};
		glm::vec4 borderColor {};
		float borderThickness {};
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
