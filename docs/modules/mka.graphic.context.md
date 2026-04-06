# Module `mka.graphic.context`

[⬅ Back to index](./README.md) · [➡ Next: window](./mka.graphic.window.md)

`mka.graphic.context` defines the rendering backend abstraction. A context instance is responsible for backend initialization, making itself current on the calling thread, and swapping buffers.

## Enums

### `API`
- `API::None`: no backend selected.
- `API::OpenGL`: OpenGL backend.
- `API::Vulkan`: Vulkan backend (placeholder).
- `API::DirectX`: DirectX backend (placeholder).
- `API::Metal`: Metal backend (placeholder).

### `Loader`
- `Loader::None`: no loader.
- `Loader::Glad`: GLAD loader.
- `Loader::Glew`: GLEW loader (placeholder).

## Class `Context`

Pure interface for concrete backend contexts.

### Methods
- `bool init(GLFWwindow* window)`: initialize backend state from a native GLFW window.
- `std::string getName() const`: backend display name, useful in logs.
- `API getAPI() const`: returns the backend family.
- `void makeCurrent()`: activate the context on current thread.
- `void swapBuffers()`: present rendered content.

## Factory function

### `std::unique_ptr<Context> createContext(API api, Loader loader = Loader::None)`
Creates a concrete implementation from (`API`, `Loader`) pair.

```cpp
auto ctx = mka::graphic::createContext(
    mka::graphic::API::OpenGL,
    mka::graphic::Loader::Glad
);
if (!ctx) {
    // unsupported or invalid combination
}
```

## Current implementation status
- OpenGL + GLAD: implemented.
- OpenGL + GLEW: not implemented yet.
- Vulkan / DirectX / Metal: stub classes returning `false` from `init()`.
