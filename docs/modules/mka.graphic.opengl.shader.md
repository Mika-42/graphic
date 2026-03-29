# Module `mka.graphic.opengl.shader`

[⬅ Retour index](./README.md) · [⬅ Module uniform](./mka.graphic.opengl.uniform.md) · [➡ Module renderer](./mka.graphic.opengl.renderer.md)

Ce module encapsule la gestion d’un programme shader OpenGL (compile, attach, link, use, set uniforms).

## Enum

### `ShaderType`
- `ShaderType::Vertex` : shader de sommet.
- `ShaderType::Fragment` : shader de fragment.
- `ShaderType::Geometry` : shader de géométrie.

## Classe `Shader`

### Méthodes

#### `Shader()`
Crée un objet programme OpenGL.

```cpp
mka::graphic::gl::Shader shader;
```

#### `~Shader()`
Libère le programme OpenGL.

```cpp
{
    mka::graphic::gl::Shader tmp;
} // destruction automatique
```

#### `void use() const`
Active le programme pour les draw calls suivants.

```cpp
shader.use();
```

#### `template<typename T> void set(const std::string& uniform_variable, T value) const`
Récupère la location puis écrit la valeur via `glUniform` du module uniform.

```cpp
shader.set("uOpacity", 0.75f);
shader.set("uProjection", glm::mat4{1.0f});
```

#### `std::string addScript(const std::string& shaderCode, ShaderType type) const`
Compile et attache un script shader; retourne un log en cas d’erreur.

```cpp
auto vLog = shader.addScript(vertexSource, mka::graphic::gl::ShaderType::Vertex);
auto fLog = shader.addScript(fragmentSource, mka::graphic::gl::ShaderType::Fragment);
```

#### `std::string link() const`
Link le programme et retourne le log linker si échec.

```cpp
auto linkLog = shader.link();
if (!linkLog.empty()) {
    // debug link errors
}
```
