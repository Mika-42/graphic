# Module `mka.graphic.opengl.renderer`

[⬅ Retour index](./README.md) · [⬅ Module shader](./mka.graphic.opengl.shader.md) · [⬅ Module sanitize](./mka.graphic.sanitize.md)

## Types exportés
- `struct Rectangle`
- `struct Text`
- `template<size_t MAX_RECTANGLE_COUNT> class Renderer`

## Fonctions module
- `uint64_t loadTexture(const char* path)`

## Méthodes clés de `Renderer`
- `Renderer()` / `~Renderer()`
- `Rectangle* add(Rectangle&& r)`
- `size_t add(const Text& text)`
- `void draw(const glm::mat4 projection)`
- `void setBackgroundColor(const glm::vec4& color)`

## Exemple d'usage
```cpp
mka::graphic::gl::Renderer<4096> renderer;
renderer.setBackgroundColor({1, 1, 1, 1});

renderer.add({
    .geometry = {100, 100, 300, 120},
    .backgroundColorA = {0.2f, 0.2f, 0.8f, 1.0f},
    .backgroundColorB = {0.1f, 0.8f, 0.8f, 1.0f},
    .gradientAngle = 35.0f
});

renderer.add({
    .content = "Audio Engine",
    .font = "./assets/Inter-Regular.ttf",
    .color = {0, 0, 0, 1},
    .position = {110, 150},
    .fontSize = 42
});

renderer.draw(projection);
```
