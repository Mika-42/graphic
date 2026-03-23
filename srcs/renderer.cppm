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
					vec4 borderThickness;
				};

				layout(std430, binding = 0) buffer Rects {
					Rect rects[];
				};

				out vec2 uv;
				out vec4 fillColor;

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

					vec2 pos = r.geometry.xy + aPos * r.geometry.zw;

					gl_Position = vec4(pos, 0.0, 1.0);
				}
			)";

const auto fs = R"(
				#version 460 core

				in vec2 uv;
				in vec4 fillColor;

				out vec4 FragColor;

				void main() {
					FragColor = fillColor;
				}
			)";

//OpenGL
export namespace mka::graphic::gl {
	
	struct Rectangle {		
		glm::vec4 geometry {}; //x, y, w, h
		glm::vec4 radius {};
		glm::vec4 fillColor {};
		glm::vec4 borderColor {};
		glm::vec4 borderThickness {};
	};

	class Renderer {
		
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

			void add(Rectangle r) {
				rectangles.emplace_back(std::move(r));
			}

			void draw() {
				if (rectangles.empty()) return;

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
				glBindVertexArray(vao);

				glDrawArraysInstanced(GL_TRIANGLES, 0, 6, rectangles.size());

			rectangles.clear();
			}

		private:
			std::vector<Rectangle> rectangles;
			Shader shader;

			GLuint vao = 0;
			GLuint ssbo = 0;
			size_t capacity = 1024;
	};
}
