# Module `mka.graphic.opengl.uniform`

[⬅ Back to index](./README.md) · [➡ Next: shader](./mka.graphic.opengl.shader.md)

Type-safe uniform write dispatch for OpenGL program uniforms.

## API
- `template<typename T> struct Uniform;`
- `template<typename T> void glUniform(GLuint program, GLint location, const T& value);`

If a `Uniform<T>` specialization does not exist, compilation fails explicitly.

## Supported types
- Scalars: `float`, `int`, `unsigned int`, `bool`
- Vectors: `glm::vec2/3/4`, `glm::ivec2/3/4`, `glm::uvec2/3/4`
- Matrices: `glm::mat2`, `glm::mat3`, `glm::mat4`
- Arrays: `std::vector<float>`, `std::vector<glm::vec3>`, `std::vector<glm::vec4>`

## Example
```cpp
GLuint program = /* linked program */;
GLint locProj = glGetUniformLocation(program, "uProjection");
GLint locTint = glGetUniformLocation(program, "uTint");

mka::graphic::gl::glUniform(program, locProj, glm::mat4{1.0f});
mka::graphic::gl::glUniform(program, locTint, glm::vec4{1, 0.5f, 0.2f, 1});
```
