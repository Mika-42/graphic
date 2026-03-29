# Module `mka.graphic.opengl.shader`

[⬅ Retour index](./README.md) · [⬅ Module uniform](./mka.graphic.opengl.uniform.md) · [➡ Module renderer](./mka.graphic.opengl.renderer.md)

## Types
- `enum class ShaderType`
- `class Shader`

## Méthodes de `Shader`
- `Shader()`
- `~Shader()`
- `void use() const`
- `template<typename T> void set(const std::string& uniform_variable, T value) const`
- `std::string addScript(const std::string& shaderCode, ShaderType type) const`
- `std::string link() const`

## Exemple d'usage
```cpp
mka::graphic::gl::Shader shader;
shader.addScript(vertexSource, mka::graphic::gl::ShaderType::Vertex);
shader.addScript(fragmentSource, mka::graphic::gl::ShaderType::Fragment);
shader.link();
shader.use();
shader.set("uTime", 1.0f);
```
