/**
 * @file renderer.cppm
 * @brief C++20 module implementation for `renderer`.
 */
module;
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "debug.hpp"
#include "glad.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <glm/glm.hpp>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <iomanip>
#include <set>

export module mka.graphic.opengl.renderer;
export import mka.graphic.opengl.rectangle; // TODO rm export
export import mka.graphic.opengl.text;      // TODO rm export
import mka.graphic.opengl.text.rasterizer;
import mka.graphic.opengl.shader;
import mka.graphic.sanitize;

namespace {
// Shader sources stay in C++ to keep module self-contained.
// The GLSL code is organized with helper functions so future effects can be
// added without turning `main()` into a monolith.
constexpr const char *kRendererVertexShader = R"(
		#version 460 core
		#extension GL_ARB_bindless_texture : require

		const uint FLAG_TEXT = 1u << 0;

		const float MIN_RECT_SIZE = 0.0001;

		struct Rect {
			vec4 geometry; // xy = pos, zw = size
			vec4 radius;
			vec4 backgroundColorA;
			vec4 backgroundColorB;
			vec4 borderColor;
			vec4 shadowColor;
			vec4 params;
			vec4 flags;
			vec2 shadowOffset;
			uvec2 texture;
		};

		layout(std430, binding = 0) buffer Rects {
			Rect rects[];
		};

		out vec2 localPoint;
		out vec2 rectSize;
		out vec2 texCoord;
		flat out uvec2 textureHandle;
		flat out uint v_InstanceID;
		out vec2 worldPoint;
		uniform mat4 uProjection;

		vec2 unitQuadVertex(const int vertexId) {
			vec2 quad[6] = vec2[](
				vec2(0.0, 0.0),
				vec2(1.0, 0.0),
				vec2(1.0, 1.0),
				vec2(0.0, 0.0),
				vec2(1.0, 1.0),
				vec2(0.0, 1.0)
			);
			return quad[vertexId];
		}

		vec2 computeShadowPadding(const Rect r) {
			float shadowExtent = max(r.params.y, 0.0) + max(r.params.z, 0.0);
			return abs(r.shadowOffset) + vec2(shadowExtent + 1.0);
		}

		vec2 computeTexCoord(const Rect r, vec2 aPos, vec2 localPosition) {
			if (r.flags.x > 0.0) {
				return aPos;
			}
			// Build UVs from the original rectangle space so shadow padding never
			// changes texture scale when softness/spread animate in real time.
			vec2 rectSpacePoint = localPosition + (0.5 * r.geometry.zw);
			vec2 safeRectSize = max(r.geometry.zw, vec2(MIN_RECT_SIZE));
			return rectSpacePoint / safeRectSize;
		}

		void main() {
			Rect r = rects[gl_InstanceID];
			vec2 aPos = unitQuadVertex(gl_VertexID);
 
			vec2 pad = computeShadowPadding(r);
			vec2 expandedSize = r.geometry.zw + 2.0 * pad;
			vec2 expandedPos = r.geometry.xy - pad;

			vec2 worldPos = expandedPos + aPos * expandedSize;
			localPoint = (-pad + aPos * expandedSize) - (0.5 * r.geometry.zw);
			worldPoint = worldPos;

			textureHandle = r.texture;
			rectSize = r.geometry.zw;
			texCoord = computeTexCoord(r, aPos, localPoint);
			v_InstanceID = uint(gl_InstanceID);
			gl_Position = uProjection * vec4(worldPos, 0.0, 1.0);
		}
	)";

constexpr const char *kRendererFragmentShader = R"(
		#version 460 core
		#extension GL_ARB_bindless_texture : require
		#extension GL_ARB_shader_stencil_export : enable

		const uint FLAG_TEXT = 1u << 0;
		const uint FLAG_CLIP = 1u << 1;

		const uint MAX_CLIP_DEPTH = 16;
		const uint NO_CLIP = 0xFFFFFFFFu;

		const float AA_WIDTH = 0.5;
		const float MIN_GRADIENT_EXTENT = 0.0001;
		const float MIN_SHADOW_SOFTNESS = 0.001;

		struct Rect {
			vec4 geometry; // xy = pos, zw = size
			vec4 radius;
			vec4 backgroundColorA;
			vec4 backgroundColorB;
			vec4 borderColor;
			vec4 shadowColor;
			vec4 params;
			vec4 flags;
			vec2 shadowOffset;
			uvec2 texture;
		};

		layout(std430, binding = 0) buffer Rects {
			Rect rects[];
		};
		in vec2 worldPoint;
		in vec2 localPoint;
		in vec2 rectSize;
		in vec2 texCoord;
		flat in uvec2 textureHandle;	
		flat in uint v_InstanceID;

		out vec4 FragColor;
		
		float sdRoundedBox(vec2 p, vec2 halfExtents, vec4 cornerRadius) {
			cornerRadius.xy = (p.x > 0.0) ? cornerRadius.xy : cornerRadius.zw;
			cornerRadius.x = (p.y > 0.0) ? cornerRadius.x : cornerRadius.y;
			vec2 q = abs(p) - halfExtents + cornerRadius.x;
			return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - cornerRadius.x;
		}

		vec4 computeGradientFill(vec2 p, vec4 A, vec4 B, float angle) {
			
			float angleRad = radians(angle);
			vec2 gradientDir = vec2(cos(angleRad), sin(angleRad));
			float gradientExtent = max(dot(abs(gradientDir), rectSize * 0.5), MIN_GRADIENT_EXTENT);
			float t = clamp((dot(p, gradientDir) / gradientExtent) * 0.5 + 0.5, 0.0, 1.0);
			return mix(A, B, t);
		}

		float computeShadowAlpha(vec2 p, vec4 radius, vec2 shadowOffset, float shadowSpread, float shadowSoftness) {
			vec2 shadowPoint = p - shadowOffset;
			float shadowDist = sdRoundedBox(shadowPoint, (rectSize * 0.5) + vec2(shadowSpread), radius);
			float safeSoftness = max(shadowSoftness, MIN_SHADOW_SOFTNESS);
			return clamp(1.0 - smoothstep(-safeSoftness, safeSoftness, shadowDist), 0.0, 1.0);
		}

		vec4 resolveBaseFill(vec4 gradientFill, bool hasTexture) {
			if (!hasTexture) {
				return gradientFill;
			}

			sampler2D rectTexture = sampler2D(textureHandle);
			vec4 sampled = texture(rectTexture, texCoord);
			if (sampled.a > 0.0) {
				// Glyph textures are premultiplied on upload to avoid filtering fringes.
				sampled.rgb /= sampled.a;
			}
			return vec4(mix(sampled.rgb, gradientFill.rgb, gradientFill.a), sampled.a);
		}

		float clipping() {
			int index = int(rects[v_InstanceID].flags.z);

			float alpha = 1.0;
			uint depth = 0;

			while(index >= 0 && depth <= MAX_CLIP_DEPTH) {
				
				Rect r = rects[index];

				if(r.flags.y > 0.0) {
					vec2 parentLocalPoint = worldPoint - (r.geometry.xy + 0.5 * r.geometry.zw);
					float dist = sdRoundedBox(parentLocalPoint, r.geometry.zw * 0.5, r.radius);
					alpha *= smoothstep(AA_WIDTH, -AA_WIDTH, dist);

					if (alpha < 0.01) return 0.0;
				}
				
				index = int(r.flags.z);
				
				depth++;
			}

			return alpha;
		}

		void main() {

			Rect r = rects[v_InstanceID];
			vec2 p = localPoint;

			float clipAlpha = 1.0;
			
			clipAlpha = clipping();
		    if (clipAlpha < 0.01) discard;

			vec4 gradientFill = computeGradientFill(p, r.backgroundColorA, r.backgroundColorB, r.params.x);
			float dist = sdRoundedBox(p, r.geometry.zw * 0.5, r.radius);

			bool hasTexture = any(notEqual(textureHandle, uvec2(0u)));
			bool hasRoundedCorners = any(greaterThan(r.radius, vec4(0.0)));
			bool hasBorder = r.params.w > 0.0;
			bool useShapeMask = hasRoundedCorners || hasBorder || !hasTexture;

			float shapeAlpha = smoothstep(AA_WIDTH, -AA_WIDTH, dist);
			float borderInner = smoothstep(AA_WIDTH, -AA_WIDTH, dist + r.params.w);
			float borderMask = useShapeMask ? clamp(shapeAlpha - borderInner, 0.0, 1.0) : 0.0;

			vec4 baseFill = resolveBaseFill(gradientFill, hasTexture);
			vec4 shapeColor = mix(baseFill, r.borderColor, borderMask);
			float shapeCoverage = useShapeMask ? max(shapeAlpha, borderMask) : 1.0;
			float shapeAlphaCombined = clamp(shapeColor.a * shapeCoverage, 0.0, 1.0);

			float shadowAlpha = computeShadowAlpha(p, r.radius, r.shadowOffset, r.params.z, r.params.y);
			float shadowCombinedAlpha = clamp(r.shadowColor.a * shadowAlpha, 0.0, 1.0);
			float visibleShadow = shadowCombinedAlpha * (1.0 - shapeAlphaCombined);
			float outAlpha = shapeAlphaCombined + visibleShadow;

			vec3 premulRgb = (shapeColor.rgb * shapeAlphaCombined) + (r.shadowColor.rgb * visibleShadow);
			vec3 outRgb = (outAlpha > 0.0) ? (premulRgb / outAlpha) : vec3(0.0);
			FragColor = vec4(outRgb, outAlpha * clipAlpha);
		}
	)";
} // namespace

// OpenGL
export namespace mka::graphic {

/**
 * @brief Instanced rectangle renderer with optional text support.
 */
class Renderer {
public:
  Renderer() {
    DEBUG_LOG("Renderer init started.");
    DEBUG_LOG("Vertex:" + shader.addScript(kRendererVertexShader, ShaderType::Vertex));
    DEBUG_LOG("Fragment:" + shader.addScript(kRendererFragmentShader, ShaderType::Fragment));
    shader.link();

    // empty vao (without it doesn't work...)
    glCreateVertexArrays(1, &vao);
    if (vao == 0) {
      DEBUG_LOG("glCreateVertexArrays failed: vao == 0.");
    }

    // create an ssbo
    glCreateBuffers(1, &ssbo);
    if (ssbo == 0) {
      DEBUG_LOG("glCreateBuffers failed: ssbo == 0.");
    }

    rectangles.reserve(initialSsboCapacity);
    if (!reserveSsboCapacity(initialSsboCapacity)) {
      DEBUG_LOG("Initial SSBO allocation failed.");
    }

    DEBUG_LOG("Renderer init completed.");

  }

  ~Renderer() {
    if (ssbo != 0) {
      glDeleteBuffers(1, &ssbo);
    }

    if (vao != 0) {
      glDeleteVertexArrays(1, &vao);
    }
  }

  uint32_t add(Rectangle &r) { 
	  rectangles.emplace_back(r);
	  return rectangles.size() - 1; //index
  }
  /**
   * @brief Build text using the rectangle pipeline (1 glyph == 1 rectangle).
   * @return Number of generated rectangle instances.
   */
  size_t add(const Text &text) {
    Text sanitizedText = text;
    sanitizeText(sanitizedText);

    // Text colors are now fully controlled by gradientColorA/B.
    // For a solid color, caller can simply set A == B.
    const glm::vec4 textGradientA = sanitizedText.gradientColorA;
    const glm::vec4 textGradientB = sanitizedText.gradientColorB;

    if (sanitizedText.content.empty()) {
      DEBUG_LOG("Empty text content.");
      return 0;
    }

    Rectangle glyphTemplate{};
    glyphTemplate.geometry = {sanitizedText.position.x,
                              sanitizedText.position.y, sanitizedText.fontSize,
                              sanitizedText.fontSize};
    glyphTemplate.backgroundColorA = textGradientA;
    glyphTemplate.backgroundColorB = textGradientB;
    glyphTemplate.params.x = sanitizedText.gradientAngle;
    glyphTemplate.flags.x = TEXT;

    size_t addedCount = 0;
    float cursorX = sanitizedText.position.x;
    const unsigned int pixelSize =
        static_cast<unsigned int>(sanitizedText.fontSize);
    std::vector<Rectangle> glyphRects;
    glyphRects.reserve(sanitizedText.content.size());

    TextLineMetrics lineMetrics{};
    if (!textRasterizer.getLineMetrics(sanitizedText.font, pixelSize,
                                       lineMetrics)) {
      DEBUG_LOG("Failed to get text line metrics for font: " +
                sanitizedText.font);
      return 0;
    }

    // `position` remains the top-left anchor of the text block.
    // We derive a single bottom line from font metrics so every glyph ends on
    // the same Y.
    const float lineBottomY = sanitizedText.position.y + lineMetrics.lineHeight;

    for (char c : sanitizedText.content) {
      const TextGlyph *glyph = textRasterizer.getOrCreateGlyph(
          sanitizedText.font, static_cast<unsigned char>(c), pixelSize);
      if (glyph == nullptr) {
        DEBUG_LOG("Missing glyph for codepoint: " +
                  std::to_string(static_cast<unsigned int>(
                      static_cast<unsigned char>(c))));
        break;
      }

      Rectangle glyphRect = glyphTemplate;
      glyphRect.geometry.x = cursorX + glyph->bearing.x;
      // Keep top-left rectangle placement but align all glyph bottoms on the
      // same line. This avoids per-glyph vertical drift with symbols/accented
      // characters.
      glyphRect.geometry.y = lineBottomY - glyph->size.y;
      glyphRect.geometry.z = glyph->size.x;
      glyphRect.geometry.w = glyph->size.y;

      glyphRect.texture = glyph->texture;
      glyphRect.flags.x = TEXT;
      glyphRects.push_back(std::move(glyphRect));
      // Advance comes from font metrics to preserve spacing/kerning quality.
      cursorX += glyph->advance + sanitizedText.letterSpacing;
    }

    if (glyphRects.empty()) {
      DEBUG_LOG("No glyph rectangle generated for provided text.");
      return 0;
    }

    const float angleRad = glm::radians(sanitizedText.gradientAngle);
    const glm::vec2 gradientDir = {std::cos(angleRad),
                                   std::sin(angleRad)};

    float globalMinProjection = std::numeric_limits<float>::max();
    float globalMaxProjection = std::numeric_limits<float>::lowest();

    for (const Rectangle &glyphRect : glyphRects) {
      const glm::vec2 center = {
          glyphRect.geometry.x + (glyphRect.geometry.z * 0.5f),
          glyphRect.geometry.y + (glyphRect.geometry.w * 0.5f)};
      const glm::vec2 halfSize = {glyphRect.geometry.z * 0.5f,
                                  glyphRect.geometry.w * 0.5f};
      // Keep CPU-side gradient projection identical to the fragment shader:
      // projection range must account for both axis contributions when angle is
      // diagonal.
      const float projectionExtent =
          std::max(glm::dot(glm::abs(gradientDir), halfSize), 0.0001f);
      const float centerProjection = glm::dot(center, gradientDir);
      globalMinProjection =
          std::min(globalMinProjection, centerProjection - projectionExtent);
      globalMaxProjection =
          std::max(globalMaxProjection, centerProjection + projectionExtent);
    }

    const float projectionRange =
        std::max(globalMaxProjection - globalMinProjection, 0.0001f);
    for (Rectangle &glyphRect : glyphRects) {
      const glm::vec2 center = {
          glyphRect.geometry.x + (glyphRect.geometry.z * 0.5f),
          glyphRect.geometry.y + (glyphRect.geometry.w * 0.5f)};
      const glm::vec2 halfSize = {glyphRect.geometry.z * 0.5f,
                                  glyphRect.geometry.w * 0.5f};
      const float projectionExtent =
          std::max(glm::dot(glm::abs(gradientDir), halfSize), 0.0001f);
      const float centerProjection = glm::dot(center, gradientDir);
      const float localMinProjection = centerProjection - projectionExtent;
      const float localMaxProjection = centerProjection + projectionExtent;
      const float tA = glm::clamp((localMinProjection - globalMinProjection) /
                                      projectionRange,
                                  0.0f, 1.0f);
      const float tB = glm::clamp((localMaxProjection - globalMinProjection) /
                                      projectionRange,
                                  0.0f, 1.0f);
      glyphRect.backgroundColorA = glm::mix(textGradientA, textGradientB, tA);
      glyphRect.backgroundColorB = glm::mix(textGradientA, textGradientB, tB);
      add(glyphRect);
      ++addedCount;
    }

    return addedCount;
  }

/* TODO move in a debug file for unit test
void debugChainage(const std::vector<Rectangle>& rectangles) {
    std::cout << "\n🔗 CHAÎNAGE DES RECTANGLES (taille: " << rectangles.size() << ")\n";
    std::cout << "============================================================\n";
    
    for (size_t rectIndex = 0; rectIndex < rectangles.size(); ++rectIndex) {
        const auto& rect = rectangles[rectIndex];
        
        if (rect.flags.z < 0.0f) {
            std::cout << "[" << std::setw(2) << rectIndex << "] NO_CLIP\n";
            continue;
        }
        
        std::cout << "[" << std::setw(2) << rectIndex << "] flags={" 
                  << rect.flags.x << "," << rect.flags.y << "," 
                  << rect.flags.z << "} → ";
        
        // 🔥 Parcours sécurisé de la chaîne
        std::set<int32_t> visited;  // Détection de cycles
        int32_t current = rect.flags.z;
        
        while (current >= 0 && static_cast<size_t>(current) < rectangles.size()) {
            if (visited.count(current)) {
                std::cout << "[CYCLE! " << current << "] ";
                break;
            }
            visited.insert(current);
            
            const auto& clipRect = rectangles[current];
            std::cout << current;
            
            if (clipRect.flags.y > 0.0f) {
                std::cout << "(CLIP)";
            }
            std::cout << "→";
            
            current = clipRect.flags.z;
        }
        
        std::cout << "NO_CLIP\n";
    }
    
    std::cout << "============================================================\n\n";
}
*/

  /// TODO Make it unusable by user
  /// @brief Flush batched rectangles to GPU and issue one instanced draw.
  void draw(const glm::mat4 projection) {

    glClearColor(bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    glClear(GL_COLOR_BUFFER_BIT);
 
	if (rectangles.empty()) {
      return;
    }

    if (ssbo == 0 || vao == 0) {
      DEBUG_LOG("draw aborted: invalid OpenGL objects (ssbo or vao is 0).");
      rectangles.clear();
      return;
    }

    for (auto &rect : rectangles) {
      sanitizeRectangle(rect);
    }

	//debugChainage(rectangles); //TODO enclose with test

    if (!reserveSsboCapacity(rectangles.size())) {
      DEBUG_LOG("draw aborted: failed to reserve SSBO capacity.");
      rectangles.clear();
      return;
    }

    const size_t rectBytes = rectangles.size() * sizeof(Rectangle);
    glNamedBufferSubData(ssbo, 0, rectBytes, rectangles.data());
    // Draw
    shader.use();
    shader.set("uProjection", projection);
   
	// bind ssbo
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    glBindVertexArray(vao);

	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, rectangles.size());

    rectangles.clear();
  }

  /// @brief Set clear color used by `draw`.
  void setBackgroundColor(const glm::vec4 &color) {
    bgColor = color;
    sanitizeColor(bgColor);
  }

private:
  static constexpr size_t initialSsboCapacity = 1024u;

  [[nodiscard]] bool reserveSsboCapacity(size_t requiredInstances) {
    if (requiredInstances == 0u) {
      return true;
    }
    if (requiredInstances <= ssboCapacityInstances) {
      return true;
    }

    // Grow with power-of-two strategy to keep reallocations rare in real-time
    // rendering. This avoids per-frame stalls while still accepting bursty
    // dynamic rectangle counts.
    size_t newCapacity = std::max(ssboCapacityInstances, initialSsboCapacity);
    while (newCapacity < requiredInstances) {
      if (newCapacity > (std::numeric_limits<size_t>::max() / 2u)) {
        DEBUG_LOG("reserveSsboCapacity overflow while growing capacity.");
        return false;
      }
      newCapacity *= 2u;
    }

    const size_t allocationBytes = newCapacity * sizeof(Rectangle);
    if (allocationBytes / sizeof(Rectangle) != newCapacity) {
      DEBUG_LOG("reserveSsboCapacity overflow during byte size computation.");
      return false;
    }
 
	glNamedBufferData(ssbo, allocationBytes, nullptr, GL_DYNAMIC_DRAW);
    ssboCapacityInstances = newCapacity;
    return true;
  }

  std::vector<Rectangle> rectangles;
  TextRasterizer textRasterizer{};

  glm::vec4 bgColor = glm::vec4{1.0f};
  Shader shader;

  GLuint vao = 0;
  GLuint ssbo = 0;
  size_t ssboCapacityInstances = 0;

  uint32_t currentClipIndex = NO_CLIP;
};
} // namespace mka::graphic
