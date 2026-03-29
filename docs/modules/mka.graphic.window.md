# Module `mka.graphic.window`

[⬅ Retour index](./README.md) · [⬅ Module context](./mka.graphic.context.md) · [➡ Module renderer](./mka.graphic.opengl.renderer.md)

## Types
- `enum class State`
- `class Window`

## Méthodes principales de `Window`
- `Window(size_t width, size_t height, const std::string& name, std::unique_ptr<Context> ctx)`
- `virtual void render() = 0`
- `const glm::mat4& getOrthographicProjection() const`
- `int run()`
- `const State& getState()`
- `const glm::vec2 getSize()`

## Exemple d'usage
```cpp
class App : public mka::graphic::Window {
public:
    App(std::unique_ptr<mka::graphic::Context> ctx)
        : Window(1280, 720, "App", std::move(ctx)) {}

    void render() override {
        // draw calls
    }
};
```
