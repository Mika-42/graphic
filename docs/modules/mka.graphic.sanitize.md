# Module `mka.graphic.sanitize`

[⬅ Retour index](./README.md) · [➡ Module renderer](./mka.graphic.opengl.renderer.md)

## Fonctions
- `void sanitizeGeometry(glm::vec4& g)`
- `void sanitizeRadius(glm::vec4& radius, const glm::vec2& size)`
- `void sanitizeColor(glm::vec4& color)`
- `float sanitizeFloat(float v, float minValue = 0.0f)`
- `void sanitizeBorderThickness(float& thickness)`
- `void sanitizeShadow(glm::vec2& shadowOffset, float& shadowSoftness, float& shadowSpread)`

## Exemple d'usage
```cpp
glm::vec4 color{2.0f, -1.0f, 0.4f, NAN};
mka::graphic::sanitizeColor(color);
// color devient clampée dans [0..1]
```
