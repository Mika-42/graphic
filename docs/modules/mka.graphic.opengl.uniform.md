# Module `mka.graphic.opengl.uniform`

[⬅ Retour index](./README.md) · [➡ Module shader](./mka.graphic.opengl.shader.md)

Ce module fournit le dispatch typé des écritures d’uniforms OpenGL via des spécialisations de `Uniform<T>`.

## API

### `template<typename T> struct Uniform;`
Trait spécialisé par type C++ pour mapper vers l’appel OpenGL adéquat.

### `template<typename T> void glUniform(GLuint program, GLint location, const T& value)`
Point d’entrée générique utilisé par le module Shader.

## Types supportés
- Scalaires: `float`, `int`, `unsigned int`, `bool`
- Vecteurs float/int/uint: `glm::vec*`, `glm::ivec*`, `glm::uvec*`
- Matrices: `glm::mat2`, `glm::mat3`, `glm::mat4`
- Tableaux: `std::vector<float>`, `std::vector<glm::vec3>`, `std::vector<glm::vec4>`

## Exemples (usage de `glUniform` avec plusieurs types)
```cpp
GLuint program = /* linked program */;
GLint locTime = glGetUniformLocation(program, "uTime");
GLint locProj = glGetUniformLocation(program, "uProjection");
GLint locColor = glGetUniformLocation(program, "uColor");

mka::graphic::gl::glUniform(program, locTime, 1.0f);                 // float
mka::graphic::gl::glUniform(program, locProj, glm::mat4{1.0f});      // mat4
mka::graphic::gl::glUniform(program, locColor, glm::vec4{1,0,0,1});  // vec4
```
