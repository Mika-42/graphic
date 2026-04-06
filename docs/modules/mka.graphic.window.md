# Module `mka.graphic.window`

[⬅ Back to index](./README.md) · [⬅ Previous: context](./mka.graphic.context.md) · [➡ Next: renderer](./mka.graphic.opengl.renderer.md)

This module exposes the application window abstraction and the main frame loop.

## Enum `State`
- `Inited`: initialization succeeded, ready to run.
- `Running`: frame loop active.
- `Stopped`: loop exited cleanly.
- `Terminated`: initialization failed or fully shut down.

## Struct `Time`
Per-frame timing values passed to `render(...)`:
- `now`: absolute GLFW time in milliseconds.
- `delta`: frame delta in milliseconds.

## Class `Window`
`Window` is intended as a base class. You implement `render(...)` and call `run()`.

### Constructor
`Window(size_t width, size_t height, const std::string& name, std::unique_ptr<Context> ctx)`

Responsibilities:
- initialize GLFW,
- apply API-specific window hints,
- create native window,
- register callbacks (resize, cursor, scroll, mouse buttons),
- initialize context,
- enable alpha blending for the OpenGL pipeline.

### Render callback
`virtual void render(const glm::vec2& size,
                     const MouseEventView& mouse,
                     const KeyboardEventView& keyboard,
                     const Time& time) = 0;`

This callback is called once per frame while the loop is running.

### Other methods
- `const glm::mat4& getOrthographicProjection() const`: projection matrix maintained on resize.
- `int run()`: starts blocking loop and drives render flow.
- `const State& getState()`: current lifecycle state.

## Input views
`Window` passes read-only wrappers to input state:
- `KeyboardEventView`: key queries (`isPressed`, `pressedKeys`, key names).
- `MouseEventView`: button/wheel/cursor queries.

## Example
```cpp
class App : public mka::graphic::Window {
public:
    explicit App(std::unique_ptr<mka::graphic::Context> ctx)
        : Window(1280, 720, "App", std::move(ctx)) {}

    void render(const glm::vec2& size,
                const MouseEventView& mouse,
                const KeyboardEventView& keyboard,
                const Time& time) override {
        if (keyboard.isPressed(mka::graphic::Key::Escape)) {
            // example: app-specific quit logic
        }
        (void)size; (void)mouse; (void)time;
    }
};
```
