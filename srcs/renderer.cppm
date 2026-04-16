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
			vec2 shadowOffset;
			float gradientAngle;
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
		out vec4 backgroundColorA;
		out vec4 backgroundColorB;
		out float gradientAngle;
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
		out vec2 rectWorldPos;
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
			float shadowExtent = max(r.shadowSoftness, 0.0) + max(r.shadowSpread, 0.0);
			return abs(r.shadowOffset) + vec2(shadowExtent + 1.0);
		}

		vec2 computeTexCoord(const Rect r, vec2 aPos, vec2 localPosition) {
			if ((r.flags & FLAG_TEXT) != 0u) {
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
			
			rectWorldPos = r.geometry.xy + 0.5 * r.geometry.zw;

			textureHandle = r.texture;
			backgroundColorA = r.backgroundColorA;
			backgroundColorB = r.backgroundColorB;
			gradientAngle = r.gradientAngle;
			radius = r.radius;
			rectSize = r.geometry.zw;
			borderColor = r.borderColor;
			borderThickness = r.borderThickness;
			shadowColor = r.shadowColor;
			shadowOffset = r.shadowOffset;
			shadowSoftness = r.shadowSoftness;
			shadowSpread = r.shadowSpread;
			texCoord = computeTexCoord(r, aPos, localPoint);

			gl_Position = uProjection * vec4(worldPos, 0.0, 1.0);
		}
	)";

constexpr const char *kRendererFragmentShader = R"(
		#version 460 core
		#extension GL_ARB_bindless_texture : require
		#extension GL_ARB_shader_stencil_export : enable

		const uint FLAG_TEXT = 1u << 0;

		const float AA_WIDTH = 0.5;
		const float MIN_GRADIENT_EXTENT = 0.0001;
		const float MIN_SHADOW_SOFTNESS = 0.001;

		in vec2 rectWorldPos;
		in vec2 localPoint;
		in vec4 backgroundColorA;
		in vec4 backgroundColorB;
		in float gradientAngle;
		in vec4 radius;
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
		
		uniform int uClipStackSize;
	
		struct ClipStack {
			vec4 geometry;
			vec4 radius;
			vec2 center;
			vec2 halfSize;
		};

		layout(std430, binding = 1) buffer Clip {
			ClipStack clips[];
		};

		float sdRoundedBox(vec2 p, vec2 halfExtents, vec4 cornerRadius) {
			cornerRadius.xy = (p.x > 0.0) ? cornerRadius.xy : cornerRadius.zw;
			cornerRadius.x = (p.y > 0.0) ? cornerRadius.x : cornerRadius.y;
			vec2 q = abs(p) - halfExtents + cornerRadius.x;
			return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - cornerRadius.x;
		}

		vec4 computeGradientFill(vec2 p) {
			float angleRad = radians(gradientAngle);
			vec2 gradientDir = vec2(cos(angleRad), sin(angleRad));
			float gradientExtent = max(dot(abs(gradientDir), rectSize * 0.5), MIN_GRADIENT_EXTENT);
			float t = clamp((dot(p, gradientDir) / gradientExtent) * 0.5 + 0.5, 0.0, 1.0);
			return mix(backgroundColorA, backgroundColorB, t);
		}

		float computeShadowAlpha(vec2 p) {
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

		void main() {
			vec2 p = localPoint;

			vec4 gradientFill = computeGradientFill(p);
			float dist = sdRoundedBox(p, rectSize * 0.5, radius);

			bool hasTexture = any(notEqual(textureHandle, uvec2(0u)));
			bool hasRoundedCorners = any(greaterThan(radius, vec4(0.0)));
			bool hasBorder = borderThickness > 0.0;
			bool useShapeMask = hasRoundedCorners || hasBorder || !hasTexture;

			float shapeAlpha = smoothstep(AA_WIDTH, -AA_WIDTH, dist);
			float borderInner = smoothstep(AA_WIDTH, -AA_WIDTH, dist + borderThickness);
			float borderMask = useShapeMask ? clamp(shapeAlpha - borderInner, 0.0, 1.0) : 0.0;

			vec4 baseFill = resolveBaseFill(gradientFill, hasTexture);
			vec4 shapeColor = mix(baseFill, borderColor, borderMask);
			float shapeCoverage = useShapeMask ? max(shapeAlpha, borderMask) : 1.0;
			float shapeAlphaCombined = clamp(shapeColor.a * shapeCoverage, 0.0, 1.0);

			float shadowAlpha = computeShadowAlpha(p);
			float shadowCombinedAlpha = clamp(shadowColor.a * shadowAlpha, 0.0, 1.0);
			float visibleShadow = shadowCombinedAlpha * (1.0 - shapeAlphaCombined);
			float outAlpha = shapeAlphaCombined + visibleShadow;

			vec3 premulRgb = (shapeColor.rgb * shapeAlphaCombined) + (shadowColor.rgb * visibleShadow);
			vec3 outRgb = (outAlpha > 0.0) ? (premulRgb / outAlpha) : vec3(0.0);
			FragColor = vec4(outRgb, outAlpha);
		}
	)";
} // namespace

// OpenGL
export namespace mka::graphic {
/**
 * @brief Instanced rectangle renderer with optional text support.
 */
class Renderer {
private:
  struct alignas(16) ClipStack {
    glm::vec4 geometry {};
    glm::vec4 radius {};
    glm::vec2 center {};
    glm::vec2 halfSize {};
  };

  std::vector<ClipStack> clipStack;

public:
  void pushClip(const glm::vec4& geometry, const glm::vec4& radius) {
    const glm::vec2 size(geometry.z, geometry.w);
	const glm::vec2 pos(geometry.x, geometry.y);

    clipStack.emplace_back(geometry, radius, pos + 0.5f * size, 0.5f * size);
  }
 
  void popClip() {
    if (!clipStack.empty()) {
      clipStack.pop_back();
    }
  }

  void clearClip() {
	clipStack.clear();
  }

public:
  Renderer() {
    DEBUG_LOG("Renderer init started.");
    DEBUG_LOG(shader.addScript(kRendererVertexShader, ShaderType::Vertex));
    DEBUG_LOG(shader.addScript(kRendererFragmentShader, ShaderType::Fragment));
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

	// Créer un SSBO pour la stack de clips
	glCreateBuffers(1, &clipSsbo);
	if (clipSsbo == 0) {
	  DEBUG_LOG("glCreateBuffers failed: clipSsbo == 0.");
	}
	clipStack.reserve(initialSsboCapacity);
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

	if (clipSsbo != 0) {
	  glDeleteBuffers(1, &clipSsbo);
	}

    if (vao != 0) {
      glDeleteVertexArrays(1, &vao);
    }
  }

  void add(Rectangle &&r) { rectangles.emplace_back(std::move(r)); }

  void add(Rectangle &r) { rectangles.emplace_back(r); }
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
    glyphTemplate.gradientAngle = sanitizedText.gradientAngle;
    glyphTemplate.flags |= TEXT;

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
      glyphRect.flags |= TEXT;
      glyphRects.push_back(std::move(glyphRect));
      // Advance comes from font metrics to preserve spacing/kerning quality.
      cursorX += glyph->advance + sanitizedText.letterSpacing;
    }

    if (glyphRects.empty()) {
      DEBUG_LOG("No glyph rectangle generated for provided text.");
      return 0;
    }

    const float gradientAngleRad = glm::radians(sanitizedText.gradientAngle);
    const glm::vec2 gradientDir = {std::cos(gradientAngleRad),
                                   std::sin(gradientAngleRad)};

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
      add(std::move(glyphRect));
      ++addedCount;
    }

    return addedCount;
  }

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

    if (!reserveSsboCapacity(rectangles.size())) {
      DEBUG_LOG("draw aborted: failed to reserve SSBO capacity.");
      rectangles.clear();
      return;
    }

    const size_t rectBytes = rectangles.size() * sizeof(Rectangle);
    glNamedBufferSubData(ssbo, 0, rectBytes, rectangles.data());

	const size_t clipBytes = clipStack.size() * sizeof(ClipStack);
    glNamedBufferSubData(clipSsbo, 0, clipBytes, clipStack.data());

    // Draw
    shader.use();
    shader.set("uProjection", projection);
	shader.set("uClipStackSize", static_cast<int>(clipStack.size()));
   
	// bind ssbo
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, clipSsbo);
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

	const size_t allocationBytesClip = newCapacity * sizeof(ClipStack);
    if (allocationBytesClip / sizeof(ClipStack) != newCapacity) {
      DEBUG_LOG("reserveSsboCapacity overflow during byte size computation.");
      return false;
    }
    
	glNamedBufferData(ssbo, allocationBytes, nullptr, GL_DYNAMIC_DRAW);
	glNamedBufferData(clipSsbo, allocationBytesClip, nullptr, GL_DYNAMIC_DRAW);
    ssboCapacityInstances = newCapacity;
    return true;
  }

  std::vector<Rectangle> rectangles;
  TextRasterizer textRasterizer{};

  glm::vec4 bgColor = glm::vec4{1.0f};
  Shader shader;

  GLuint vao = 0;
  GLuint ssbo = 0;
  GLuint clipSsbo = 0;
  size_t ssboCapacityInstances = 0;
};
} // namespace mka::graphic
