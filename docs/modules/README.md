# `mka.graphic` modules documentation

This folder is the long-term reference for the engine module API.

## What you can find here
- Full module map and responsibilities.
- Runtime lifecycle (context -> window -> renderer).
- Input and rendering data flow.
- Practical usage snippets for each exported module.
- Limitations and extension points for future work.

## Module index
- [mka.graphic.context](./mka.graphic.context.md)
- [mka.graphic.window](./mka.graphic.window.md)
- [mka.graphic.keyboard](./mka.graphic.keyboard.md)
- [mka.graphic.mouse](./mka.graphic.mouse.md)
- [mka.graphic.sanitize](./mka.graphic.sanitize.md)
- [mka.graphic.opengl.uniform](./mka.graphic.opengl.uniform.md)
- [mka.graphic.opengl.shader](./mka.graphic.opengl.shader.md)
- [mka.graphic.opengl.renderer](./mka.graphic.opengl.renderer.md)

## Runtime architecture (quick mental model)
1. `createContext(API, Loader)` builds a backend context object.
2. `Window` configures GLFW according to the selected API.
3. `Window::run()` drives one render frame per loop iteration:
   - poll events,
   - sync keyboard state,
   - compute timing,
   - call user `render(size, mouse, keyboard, time)`,
   - present buffers.
4. `Renderer<MAX_RECTANGLE_COUNT>` consumes draw commands and flushes once per frame.

## Minimal end-to-end example
```cpp
#include <memory>
#include <glm/glm.hpp>

import mka.graphic.window;
import mka.graphic.opengl.renderer;

class MinimalApp : public mka::graphic::Window {
public:
    explicit MinimalApp(std::unique_ptr<mka::graphic::Context> ctx)
        : Window(1280, 720, "Minimal", std::move(ctx)) {}

    void render(const glm::vec2& size,
                const MouseEventView& mouse,
                const KeyboardEventView& keyboard,
                const Time& time) override {
        (void)size; (void)mouse; (void)keyboard; (void)time;

        renderer.setBackgroundColor({0.08f, 0.08f, 0.10f, 1.0f});
        renderer.add({
            .geometry = {120, 90, 320, 180},
            .radius = {20, 20, 20, 20},
            .backgroundColorA = {0.18f, 0.35f, 0.8f, 1.0f},
            .backgroundColorB = {0.10f, 0.7f, 0.65f, 1.0f},
            .gradientAngle = 30.0f
        });
        renderer.draw(getOrthographicProjection());
    }

private:
    mka::graphic::gl::Renderer<2048> renderer;
};

int main() {
    auto ctx = mka::graphic::createContext(
        mka::graphic::API::OpenGL,
        mka::graphic::Loader::Glad
    );
    if (!ctx) return 1;

    MinimalApp app(std::move(ctx));
    return app.run();
}
```

## Build and run notes
- OpenGL path currently relies on GLAD loader (`Loader::Glad`).
- Vulkan/DirectX/Metal context classes are placeholders for now.
- The renderer expects support for bindless texture functions (`ARB_bindless_texture`).
