module;
#include <cstdint>
#include <ft2build.h>
#include <string>
#include <unordered_map>
#include <vector>
#include FT_FREETYPE_H
#include "glad.h"
#include <glm/glm.hpp>

export module mka.graphic.opengl.text.rasterizer;
import mka.graphic.opengl.text;
import mka.graphic.log;

export namespace mka::graphic {

    struct TextGlyph {
        glm::uvec2 texture{};
        glm::vec2 size{};
        glm::vec2 bearing{};
        float advance{};
    };

    struct TextLineMetrics {
        float ascender{};
        float descender{};
        float lineHeight{};
    };

    /**
     * @brief Owns FreeType state and glyph cache for text rendering.
     *
     * Text primitive (content/params) stays independent while this class
     * handles expensive rasterization + GPU texture lifetime.
     */
    class TextRasterizer {
    public:
        TextRasterizer() {
            if (FT_Init_FreeType(&ftLibrary) != 0) {
                Log::error("TextRasterizer(), FreeType init failed.");
                ftLibrary = nullptr;
            }
        }

        ~TextRasterizer() {
            for (auto &[_, cache] : fontCaches) {
                for (const CachedGlyph &glyph : cache.glyphs) {
                    if (glyph.textureHandle != 0) {
                        glMakeTextureHandleNonResidentARB(glyph.textureHandle);
                    }
                    if (glyph.textureId != 0u) {
                        glDeleteTextures(1, &glyph.textureId);
                    }
                }

                if (cache.face != nullptr) {
                    FT_Done_Face(cache.face);
                    cache.face = nullptr;
                }
            }

            if (ftLibrary != nullptr) {
                FT_Done_FreeType(ftLibrary);
                ftLibrary = nullptr;
            }
        }

        [[nodiscard]] bool getLineMetrics(const std::string &fontPath, unsigned int pixelSize,
                                          TextLineMetrics &outMetrics) {
            FontCache *cache = getOrCreateFontCache(fontPath);
            if (cache == nullptr) {
                return false;
            }
            if (!setPixelSize(*cache, pixelSize)) {
                return false;
            }
            outMetrics.ascender = static_cast<float>(cache->face->size->metrics.ascender >> 6);
            outMetrics.descender = static_cast<float>(cache->face->size->metrics.descender >> 6);
            outMetrics.lineHeight = outMetrics.ascender - outMetrics.descender;
            return true;
        }

        [[nodiscard]] const TextGlyph *getOrCreateGlyph(const std::string &fontPath, std::uint32_t codepoint,
                                                        std::uint32_t pixelSize) {
            FontCache *cache = getOrCreateFontCache(fontPath);
            if (cache == nullptr) {
                return nullptr;
            }
            const std::uint64_t glyphKey = (static_cast<std::uint64_t>(pixelSize) << 32u) | codepoint;
            if (const auto it = cache->glyphsByKey.find(glyphKey); it != cache->glyphsByKey.end()) {
                return &cache->glyphs[it->second].payload;
            }

            if (!setPixelSize(*cache, pixelSize)) {
                return nullptr;
            }
            if (FT_Load_Char(cache->face, codepoint, FT_LOAD_RENDER) != 0) {
                Log::warn("getOrCreateGlyph(...) aborted, failed to load char for codepoint {}.", codepoint);
                return nullptr;
            }

            const FT_GlyphSlot glyph = cache->face->glyph;
            if (glyph == nullptr) {
                Log::warn("getOrCreateGlyph(...) aborted, glyph is null.");
                return nullptr;
            }

            const FT_Bitmap &bitmap = glyph->bitmap;
            const std::size_t pixelCount = static_cast<std::size_t>(bitmap.width) * bitmap.rows;
            if (bitmap.width > 0 && bitmap.rows > 0 && pixelCount / bitmap.width != bitmap.rows) {
				Log::warn("getOrCreateGlyph(...) aborted, glyph bitmap size overflow detected.");
                return nullptr;
            }
            if (bitmap.buffer == nullptr && pixelCount > 0) {
				Log::warn("getOrCreateGlyph(...) aborted, glyph bitmap buffer is null.");
                return nullptr;
            }

            CachedGlyph newGlyph{};
            newGlyph.payload.size = glm::vec2(static_cast<float>(bitmap.width), static_cast<float>(bitmap.rows));
            newGlyph.payload.bearing =
                glm::vec2(static_cast<float>(glyph->bitmap_left), static_cast<float>(glyph->bitmap_top));
            newGlyph.payload.advance = static_cast<float>(glyph->advance.x >> 6);

            // Important: spaces and some control glyphs are valid but have empty
            // bitmaps. We still cache advance/bearing so layout stays correct, without
            // allocating a texture.
            if (bitmap.width > 0 && bitmap.rows > 0) {
                std::vector<std::uint8_t> rgba(pixelCount * 4u, 0u);
                for (std::int32_t y = 0; y < static_cast<std::int32_t>(bitmap.rows); ++y) {
                    for (std::int32_t x = 0; x < static_cast<std::int32_t>(bitmap.width); ++x) {
                        const std::size_t srcIndex = static_cast<std::size_t>(y) * bitmap.pitch + static_cast<std::size_t>(x);
                        const std::size_t dstIndex = (static_cast<std::size_t>(y) * bitmap.width + static_cast<std::size_t>(x)) * 4u;
                        const std::uint32_t coverage = static_cast<std::uint32_t>(bitmap.buffer[srcIndex]);
                        // Keep white premultiplied coverage so shader can tint without edge
                        // fringes.
                        rgba[dstIndex + 0u] = static_cast<std::uint8_t>(coverage);
                        rgba[dstIndex + 1u] = static_cast<std::uint8_t>(coverage);
                        rgba[dstIndex + 2u] = static_cast<std::uint8_t>(coverage);
                        rgba[dstIndex + 3u] = static_cast<std::uint8_t>(coverage);
                    }
                }

                if (!createGlyphTextureRGBA(rgba.data(), static_cast<std::int32_t>(bitmap.width), static_cast<std::int32_t>(bitmap.rows),
                                            newGlyph.textureId, newGlyph.textureHandle)) {
                    return nullptr;
                }
                newGlyph.payload.texture =
                    glm::uvec2(newGlyph.textureHandle & 0xFFFFFFFFu, newGlyph.textureHandle >> 32u);
            }

            const size_t glyphIndex = cache->glyphs.size();
            cache->glyphs.emplace_back(std::move(newGlyph));
            cache->glyphsByKey.emplace(glyphKey, glyphIndex);
            return &cache->glyphs[glyphIndex].payload;
        }

    private:
        struct CachedGlyph {
            TextGlyph payload{};
            GLuint textureId = 0u;
            uint64_t textureHandle = 0u;
        };

        struct FontCache {
            FT_Face face{};
            std::vector<CachedGlyph> glyphs;
            std::unordered_map<uint64_t, size_t> glyphsByKey;
        };

        [[nodiscard]] FontCache *getOrCreateFontCache(const std::string &fontPath) {
            if (ftLibrary == nullptr || fontPath.empty()) {
                if (ftLibrary == nullptr) {
					Log::warn("getOrCreateFontCache(...) aborted, FreeType is not initialized.");
                }
                if (fontPath.empty()) {
					Log::warn("getOrCreateFontCache(...) aborted, font path is empty.");
                }
                return nullptr;
            }

            if (const auto it = fontCaches.find(fontPath); it != fontCaches.end()) {
                return &it->second;
            }

            FT_Face face = nullptr;
            if (FT_New_Face(ftLibrary, fontPath.c_str(), 0, &face) != 0 || face == nullptr) {
				Log::warn("getOrCreateFontCache(...) aborted, failed to load font ({}).", fontPath);
                return nullptr;
            }

            FontCache cache{};
            cache.face = face;
            return &fontCaches.emplace(fontPath, std::move(cache)).first->second;
        }

        [[nodiscard]] bool setPixelSize(FontCache &cache, unsigned int pixelSize) const {
            if (FT_Set_Pixel_Sizes(cache.face, 0, pixelSize) != 0) {
				Log::warn("getOrCreateFontCache(...) aborted, failed to set pixel size ({}).", pixelSize);
                return false;
            }
            return true;
        }

        [[nodiscard]] bool createGlyphTextureRGBA(const unsigned char *rgbaPixels, int width, int height,
                                                  GLuint &outTextureId, uint64_t &outTextureHandle) const {
            if (rgbaPixels == nullptr || width <= 0 || height <= 0) {
                Log::warn("createGlyphTextureRGBA(...) abort, invalid glyph texture input.");
                return false;
            }

            GLuint tex = 0;
            glCreateTextures(GL_TEXTURE_2D, 1, &tex);
            if (tex == 0) {
                Log::warn("createGlyphTextureRGBA(...) abort, OpenGL failed to create texture.");
                return false;
            }

            glTextureStorage2D(tex, 1, GL_RGBA8, width, height);
            glTextureSubImage2D(tex, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels);
            glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            const std::size_t handle = glGetTextureHandleARB(tex);
            if (handle == 0) {
                Log::warn("createGlyphTextureRGBA(...) abort, OpenGL failed to get texture handle ARB.");
                glDeleteTextures(1, &tex);
                return false;
            }
            glMakeTextureHandleResidentARB(handle);

            outTextureId = tex;
            outTextureHandle = handle;
            return true;
        }

        FT_Library ftLibrary{};
        std::unordered_map<std::string, FontCache> fontCaches;
    };
} // namespace mka::graphic
