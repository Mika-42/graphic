# Module `mka.graphic.opengl.uniform`

[⬅ Retour index](./README.md) · [➡ Module shader](./mka.graphic.opengl.shader.md)

## API
- `template<typename T> struct Uniform;` (spécialisations internes)
- `template<typename T> void glUniform(GLuint program, GLint location, const T& value)`

## Types supportés (extraits)
- Scalaires: `float`, `int`, `unsigned int`, `bool`
- Vecteurs GLM: `glm::vec*`, `glm::ivec*`, `glm::uvec*`
- Matrices GLM: `glm::mat2`, `glm::mat3`, `glm::mat4`
- Tableaux: `std::vector<float>`, `std::vector<glm::vec3>`, `std::vector<glm::vec4>`

## Exemple d'usage
```cpp
GLuint program = /* linked program */;
GLint loc = glGetUniformLocation(program, "uProjection");
glm::mat4 proj{1.0f};
mka::graphic::gl::glUniform(program, loc, proj);
```
