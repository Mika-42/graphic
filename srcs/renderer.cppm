module;
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "glad.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <array>
#include <cmath>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef DEBUG
	#include <iostream>
		#define DEBUG_LOG(x) do { std::cerr << (x) << '\n'; } while(0)
	#else
		#define DEBUG_LOG(x) do { (void)(x); } while(0)
#endif

export module mka.graphic.opengl.renderer;
import mka.graphic.opengl.shader;
import mka.graphic.sanitize;

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
					uint flags;
					uint _pad;
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
					if((r.flags & (1u << 0)) != 0u) {
						texCoord = aPos;
					} else {
						// Build UVs from the *original* rectangle space so shadow padding
						// never changes the apparent texture scale when softness varies.
						vec2 rectSpacePoint = localPoint + (0.5 * r.geometry.zw);
						vec2 safeRectSize = max(r.geometry.zw, vec2(0.0001));
						texCoord = rectSpacePoint / safeRectSize;
					}	
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
					bool hasTexture = any(notEqual(textureHandle, uvec2(0u)));
					bool hasRoundedCorners = any(greaterThan(radius, vec4(0.0)));
					bool hasBorder = borderThickness > 0.0;
					// Text glyph rectangles are textured quads with zero radius and zero border.
					// In that case we bypass SDF clipping to avoid trimming edge pixels.
					bool useShapeMask = hasRoundedCorners || hasBorder || !hasTexture;
					
					float aa = 0.5;
					float shapeAlpha = smoothstep(aa, -aa, dist); 

					float borderInner = smoothstep(aa, -aa, dist + borderThickness);
				    float borderOuter = shapeAlpha;
					float borderMask = useShapeMask ? clamp(borderOuter - borderInner, 0.0, 1.0) : 0.0;

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
					if (hasTexture) {
						sampler2D rectTexture = sampler2D(textureHandle);
						vec4 sampled = texture(rectTexture, texCoord);
						// Glyph textures are uploaded in premultiplied alpha to avoid edge fringes
						// during linear filtering; convert back to straight alpha for this shader path.
						if (sampled.a > 0.0) {
							sampled.rgb /= sampled.a;
						}
						baseFill = mix(sampled, fillColor, fillColor.a);
					}

					vec4 shapeColor = mix(baseFill, borderColor, borderMask);
					float shapeCoverage = useShapeMask ? max(shapeAlpha, borderMask) : 1.0;
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
	
	constexpr uint32_t TEXT = 1u << 0;

	export struct alignas(16) Rectangle {
		glm::vec4 geometry {}; //x, y, w, h
		glm::vec4 radius {};
		glm::vec4 backgroundColor {};
		glm::vec4 borderColor {};
		glm::vec4 shadowColor {};
		glm::vec2 shadowOffset {};
		float shadowSoftness {};
		float shadowSpread {};
		float borderThickness {};
		uint64_t texture {};
		uint32_t flags {};
		uint32_t _pad {};
	};

	export struct Text {
		std::string content {};
		std::string font {};
		glm::vec4 color {};
		glm::vec2 position {};
		float fontSize {};
		float letterSpacing {};
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

	void sanitizeRectangle(Rectangle &r) {
		sanitizeGeometry(r.geometry);
		sanitizeRadius(r.radius, glm::vec2(r.geometry.z, r.geometry.w));
		sanitizeColor(r.backgroundColor);
		sanitizeColor(r.borderColor);
		sanitizeColor(r.shadowColor);
		sanitizeShadow(r.shadowOffset, r.shadowSoftness, r.shadowSpread);
		sanitizeBorderThickness(r.borderThickness);
	}

	void sanitizeText(Text &t) {
		sanitizeColor(t.color);	
		t.fontSize = sanitizeFloat(t.fontSize, 0.0f);
		t.letterSpacing = sanitizeFloat(t.letterSpacing, 0.0f);

		if (!std::isfinite(t.position.x)) t.position.x = 0.0f;
		if (!std::isfinite(t.position.y)) t.position.y = 0.0f;
	}

	export template<size_t MAX_RECTANGLE_COUNT> class Renderer {
		
		public:

			Renderer() {
				DEBUG_LOG(shader.addScript(vs, ShaderType::Vertex)); 
				DEBUG_LOG(shader.addScript(fs, ShaderType::Fragment)); 
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

				if (FT_Init_FreeType(&ftLibrary) != 0) {
					DEBUG_LOG("FreeType init failed");
					ftLibrary = nullptr;
				}
			}
			
			~Renderer() {
				for (auto& [_, cache] : fontCaches) {
					if (cache.face != nullptr) {
						FT_Done_Face(cache.face);
						cache.face = nullptr;
					}
				}
				if (ftLibrary != nullptr) {
					FT_Done_FreeType(ftLibrary);
					ftLibrary = nullptr;
				}

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

			// Build text using the same rectangle pipeline: one character = one rectangle instance.
			// Returns the number of generated rectangles so the caller can detect truncation.
			size_t add(const Text& text) {
				Text sanitizedText = text;
				sanitizeText(sanitizedText);

				if (sanitizedText.content.empty()) {
					DEBUG_LOG("Empty text content.");
					return 0;
				}

				Rectangle glyphTemplate {};
				glyphTemplate.geometry = {
					sanitizedText.position.x,
					sanitizedText.position.y,
					sanitizedText.fontSize,
					sanitizedText.fontSize
				};
				glyphTemplate.flags |= TEXT;

				size_t addedCount = 0;
				float cursorX = sanitizedText.position.x;
				const unsigned int pixelSize = static_cast<unsigned int>(sanitizedText.fontSize);

				FontCache* fontCache = getOrCreateFontCache(sanitizedText.font);
				if (fontCache == nullptr) {
					return 0;
				}
				if (FT_Set_Pixel_Sizes(fontCache->face, 0, pixelSize) != 0) {
					return 0;
				}

				// `position` remains the top-left anchor of the text block.
				// We derive a single bottom line from font metrics so every glyph ends on the same Y.
				const float ascender = static_cast<float>(fontCache->face->size->metrics.ascender >> 6);
				const float descender = static_cast<float>(fontCache->face->size->metrics.descender >> 6);
				const float lineHeight = ascender - descender;
				const float lineBottomY = sanitizedText.position.y + lineHeight;

				for (char c : sanitizedText.content) {
					if (rectangleCount >= MAX_RECTANGLE_COUNT) {
						break;
					}

					const CachedGlyph* glyph = getOrCreateGlyph(
						*fontCache,
						static_cast<unsigned char>(c),
						pixelSize,
						sanitizedText.color
					);
					if (glyph == nullptr) {
						break;
					}

					Rectangle glyphRect = glyphTemplate;
					glyphRect.geometry.x = cursorX + glyph->bearing.x;
					// Keep top-left rectangle placement but align all glyph bottoms on the same line.
					// This avoids per-glyph vertical drift with symbols/accented characters.
					glyphRect.geometry.y = lineBottomY - glyph->size.y;
					glyphRect.geometry.z = glyph->size.x;
					glyphRect.geometry.w = glyph->size.y;
					
					glyphRect.texture = glyph->texture;
					glyphRect.flags |= TEXT;
					if (add(std::move(glyphRect)) == nullptr) {
						break;
					}

					++addedCount;
					// Advance comes from font metrics to preserve spacing/kerning quality.
					cursorX += glyph->advance + sanitizedText.letterSpacing;
				}

				return addedCount;
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
			struct CachedGlyph {
				uint64_t texture {};
				glm::vec2 size {};
				glm::vec2 bearing {};
				float advance {};
			};

			struct FontCache {
				FT_Face face {};
				std::unordered_map<uint64_t, CachedGlyph> glyphs;
			};

			[[nodiscard]] uint64_t createGlyphTextureRGBA(
				const unsigned char* rgbaPixels,
				int width,
				int height
			) const {
				if (rgbaPixels == nullptr || width <= 0 || height <= 0) {
					return 0;
				}

				GLuint tex = 0;
				glCreateTextures(GL_TEXTURE_2D, 1, &tex);
				glTextureStorage2D(tex, 1, GL_RGBA8, width, height);
				glTextureSubImage2D(tex, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels);
				glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				uint64_t handle = glGetTextureHandleARB(tex);
				glMakeTextureHandleResidentARB(handle);
				return handle;
			}

			FontCache* getOrCreateFontCache(const std::string& fontPath) {
				if (ftLibrary == nullptr || fontPath.empty()) {
					return nullptr;
				}

				if (const auto it = fontCaches.find(fontPath); it != fontCaches.end()) {
					return &it->second;
				}

				FT_Face face = nullptr;
				if (FT_New_Face(ftLibrary, fontPath.c_str(), 0, &face) != 0) {
					DEBUG_LOG("Failed to load TTF font: " + fontPath);
					return nullptr;
				}

				FontCache cache {};
				cache.face = face;
				return &fontCaches.emplace(fontPath, std::move(cache)).first->second;
			}

			const CachedGlyph* getOrCreateGlyph(
				FontCache& fontCache,
				unsigned int codepoint,
				unsigned int pixelSize,
				const glm::vec4& color
			) {
				const uint64_t glyphKey = (static_cast<uint64_t>(pixelSize) << 32u) | codepoint;
				if (const auto it = fontCache.glyphs.find(glyphKey); it != fontCache.glyphs.end()) {
					return &it->second;
				}

				if (FT_Set_Pixel_Sizes(fontCache.face, 0, pixelSize) != 0) {
					return nullptr;
				}
				if (FT_Load_Char(fontCache.face, codepoint, FT_LOAD_RENDER) != 0) {
					return nullptr;
				}

				const FT_GlyphSlot glyph = fontCache.face->glyph;
				const FT_Bitmap& bitmap = glyph->bitmap;

				std::vector<unsigned char> rgba(static_cast<size_t>(bitmap.width) * bitmap.rows * 4u, 0u);
				const unsigned char r = static_cast<unsigned char>(color.r * 255.0f);
				const unsigned char g = static_cast<unsigned char>(color.g * 255.0f);
				const unsigned char b = static_cast<unsigned char>(color.b * 255.0f);
				const unsigned char aScale = static_cast<unsigned char>(color.a * 255.0f);

				for (int y = 0; y < static_cast<int>(bitmap.rows); ++y) {
					for (int x = 0; x < static_cast<int>(bitmap.width); ++x) {
						const size_t srcIndex = static_cast<size_t>(y) * bitmap.pitch + static_cast<size_t>(x);
						const size_t dstIndex = (static_cast<size_t>(y) * bitmap.width + static_cast<size_t>(x)) * 4u;
						const unsigned int coverage = static_cast<unsigned int>(bitmap.buffer[srcIndex]);
						const unsigned int alpha = (coverage * aScale) / 255u;
						// Store glyphs in premultiplied alpha so bilinear filtering does not
						// blend colored text edges with transparent black texels.
						rgba[dstIndex + 0u] = static_cast<unsigned char>((static_cast<unsigned int>(r) * alpha) / 255u);
						rgba[dstIndex + 1u] = static_cast<unsigned char>((static_cast<unsigned int>(g) * alpha) / 255u);
						rgba[dstIndex + 2u] = static_cast<unsigned char>((static_cast<unsigned int>(b) * alpha) / 255u);
						// Alpha from glyph coverage keeps antialiasing from FreeType rasterization.
						rgba[dstIndex + 3u] = static_cast<unsigned char>(alpha);
					}
				}

				CachedGlyph cached {};
				cached.texture = createGlyphTextureRGBA(
					rgba.empty() ? nullptr : rgba.data(),
					static_cast<int>(bitmap.width),
					static_cast<int>(bitmap.rows)
				);
				cached.size = glm::vec2(static_cast<float>(bitmap.width), static_cast<float>(bitmap.rows));
				cached.bearing = glm::vec2(static_cast<float>(glyph->bitmap_left), static_cast<float>(glyph->bitmap_top));
				cached.advance = static_cast<float>(glyph->advance.x >> 6);

				return &fontCache.glyphs.emplace(glyphKey, std::move(cached)).first->second;
			}

			std::array<Rectangle, MAX_RECTANGLE_COUNT> rectangles;
			size_t rectangleCount = 0;
			FT_Library ftLibrary {};
			std::unordered_map<std::string, FontCache> fontCaches;

			glm::vec4 bgColor = glm::vec4{1.0f};
			Shader shader;

			GLuint vao = 0;
			GLuint ssbo = 0;
	};
}
