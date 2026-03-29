# Module `mka.graphic.sanitize`

[⬅ Retour index](./README.md) · [➡ Module renderer](./mka.graphic.opengl.renderer.md)

Ce module centralise la **normalisation/sécurisation** des paramètres de rendu avant envoi GPU.

## Fonctions

### `void sanitizeGeometry(glm::vec4& g)`
Valide `x, y, w, h`, remplace les non-finis, et force `w/h >= 0`.

```cpp
glm::vec4 g{10.0f, NAN, -20.0f, 80.0f};
mka::graphic::sanitizeGeometry(g);
```

### `void sanitizeRadius(glm::vec4& radius, const glm::vec2& size)`
Clamp le rayon des coins dans `[0, min(size.x, size.y)/2]`.

```cpp
glm::vec4 radius{40, 30, 80, 10};
glm::vec2 size{100, 60};
mka::graphic::sanitizeRadius(radius, size);
```

### `void sanitizeColor(glm::vec4& color)`
Clamp `rgba` dans `[0..1]` et remplace les valeurs non-finies.

```cpp
glm::vec4 color{2.0f, -1.0f, 0.4f, NAN};
mka::graphic::sanitizeColor(color);
```

### `float sanitizeFloat(float v, float minValue = 0.0f)`
Renvoie une valeur finie et au moins égale à `minValue`.

```cpp
float safeSoftness = mka::graphic::sanitizeFloat(NAN, 0.001f);
```

### `void sanitizeBorderThickness(float& thickness)`
Force une épaisseur de bordure valide (non négative, finie).

```cpp
float border = -5.0f;
mka::graphic::sanitizeBorderThickness(border);
```

### `void sanitizeShadow(glm::vec2& shadowOffset, float& shadowSoftness, float& shadowSpread)`
Normalise les paramètres d’ombre: offset fini, softness/spread valides.

```cpp
glm::vec2 offset{NAN, 8.0f};
float softness = -1.0f;
float spread = NAN;
mka::graphic::sanitizeShadow(offset, softness, spread);
```
