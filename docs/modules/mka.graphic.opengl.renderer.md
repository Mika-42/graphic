# Module `mka.graphic.opengl.renderer`

[⬅ Back to index](./README.md) · [⬅ Previous: shader](./mka.graphic.opengl.shader.md) · [⬅ Previous: sanitize](./mka.graphic.sanitize.md)

Instanced OpenGL renderer for 2D rectangles and text glyphs.

## Constant
- `TEXT = 1u << 0`: marks glyph-generated rectangle instances.

## Exported structures

### `Rectangle`
GPU payload mirrored by GLSL SSBO struct.

Important fields:
- `geometry`: `{x, y, width, height}`
- `radius`: corner radii
- `backgroundColorA/B`: gradient endpoints
- `borderColor`, `borderThickness`
- `shadowColor`, `shadowOffset`, `shadowSoftness`, `shadowSpread`
- `texture`: bindless texture handle
- `flags`: feature flags (`TEXT`, ...)

### `Text`
High-level text command converted to one rectangle per glyph.

Fields:
- `content`, `font`
- `color` or explicit `gradientColorA/B`
- `gradientAngle`
- `position`
- `fontSize`
- `letterSpacing`

## Exported helper functions
- `float distance(const Rectangle& rect, glm::vec2 point)`: signed distance utility.
- `uint64_t loadTexture(const char* path)`: loads image as RGBA texture and returns bindless handle (`0` on failure).

## Template class `Renderer<MAX_RECTANGLE_COUNT>`

### Design goals
- deterministic fixed-capacity CPU batching,
- low draw-call count (single instanced rectangle draw per frame),
- text integrated into same rectangle pipeline.

### Main methods
- `Renderer()`: compile shaders, create VAO/SSBO, init FreeType.
- `~Renderer()`: release GL resources and font caches.
- `Rectangle* add(Rectangle&& r)`: push a rectangle command.
- `size_t add(const Text& text)`: convert text to glyph rectangles.
- `void draw(const glm::mat4 projection)`: sanitize, upload, draw and clear batch.
- `void setBackgroundColor(const glm::vec4& color)`: clear color used on draw.

## Runtime sequence (per frame)
1. User enqueues rectangles/text with `add(...)`.
2. `draw(projection)` sanitizes data.
3. Batch is uploaded to SSBO.
4. Instanced draw executes rectangle shader pipeline.
5. Batch count resets for next frame.

## Known constraints
- Fixed maximum instance count from template parameter.
- Current text path handles ASCII-like per-byte iteration.
- Requires OpenGL path with bindless texture availability.
