# Module `mka.graphic.opengl.shader`

[⬅ Back to index](./README.md) · [⬅ Previous: uniform](./mka.graphic.opengl.uniform.md) · [➡ Next: renderer](./mka.graphic.opengl.renderer.md)

Wrapper around OpenGL program lifecycle and typed uniform updates.

## Enum `ShaderType`
- `Vertex`
- `Fragment`
- `Geometry`

## Class `Shader`
### Responsibilities
- create/delete OpenGL program object,
- compile and attach shader stages,
- link program,
- bind program for draw calls,
- write typed uniforms via `mka.graphic.opengl.uniform`.

### Main methods
- `Shader()`
- `~Shader()`
- `void use() const`
- `template<typename T> void set(const std::string& uniform_variable, T value) const`
- `std::string addScript(const std::string& shaderCode, ShaderType type) const`
- `std::string link() const`

### Error handling
`addScript()` and `link()` return OpenGL logs as strings. Empty string means success or no diagnostics.
