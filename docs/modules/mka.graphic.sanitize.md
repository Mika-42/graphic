# Module `mka.graphic.sanitize`

[⬅ Back to index](./README.md) · [➡ Next: renderer](./mka.graphic.opengl.renderer.md)

This module centralizes value validation before GPU upload.

## Why this module matters
Invalid values (`NaN`, infinities, negative sizes) can produce undefined behavior in shader math and hard-to-debug visual artifacts. Sanitizing once in CPU code keeps rendering deterministic.

## Functions
- `sanitizeGeometry(glm::vec4& g)`: fixes non-finite values and clamps width/height >= 0.
- `sanitizeRadius(glm::vec4& radius, const glm::vec2& size)`: clamps corner radius to valid range.
- `sanitizeColor(glm::vec4& color)`: sanitizes and clamps RGBA into [0, 1].
- `sanitizeFloat(float v, float minValue = 0.0f)`: finite + minimum clamp helper.
- `sanitizeBorderThickness(float& thickness)`: non-negative finite border width.
- `sanitizeShadow(glm::vec2& shadowOffset, float& shadowSoftness, float& shadowSpread)`: validates shadow parameters.
