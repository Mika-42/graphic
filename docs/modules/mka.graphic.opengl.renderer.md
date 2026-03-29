# Module `mka.graphic.opengl.renderer`

[⬅ Retour index](./README.md) · [⬅ Module shader](./mka.graphic.opengl.shader.md) · [⬅ Module sanitize](./mka.graphic.sanitize.md)

Ce module implémente un renderer OpenGL instancié pour rectangles et texte, adapté à un pipeline UI 2D.

## Constante
- `TEXT = 1u << 0` : flag interne indiquant une instance de glyph/texte.

## Structures exportées

### `Rectangle`
Champs:
- `geometry` : `{x, y, w, h}`
- `radius` : rayons des coins
- `backgroundColorA`, `backgroundColorB` : couleurs de gradient
- `borderColor`, `shadowColor`
- `shadowOffset`, `gradientAngle`, `shadowSoftness`, `shadowSpread`, `borderThickness`
- `texture` : handle bindless
- `flags` : bits d’options (`TEXT`...)
- `_pad` : padding d’alignement

### `Text`
Champs:
- `content` : texte UTF-8 simple (traité char par char)
- `font` : chemin de police
- `color` : couleur du texte
- `position` : ancre top-left
- `fontSize` : taille pixel
- `letterSpacing` : espacement supplémentaire

## Fonctions module

### `uint64_t loadTexture(const char* path)`
Charge une image RGBA, crée une texture GL et renvoie son handle bindless.

```cpp
uint64_t tex = mka::graphic::gl::loadTexture("./assets/logo.png");
```

## Classe template `Renderer<MAX_RECTANGLE_COUNT>`

### `Renderer()`
Compile/link les shaders, crée VAO + SSBO, initialise FreeType.

```cpp
mka::graphic::gl::Renderer<4096> renderer;
```

### `~Renderer()`
Libère caches de police, FreeType et objets GL.

```cpp
{
    mka::graphic::gl::Renderer<1024> r;
} // cleanup auto
```

### `Rectangle* add(Rectangle&& r)`
Ajoute un rectangle dans le batch CPU. Retourne `nullptr` si capacité atteinte.

```cpp
renderer.add({
    .geometry = {100, 80, 260, 120},
    .backgroundColorA = {0.2f, 0.2f, 0.8f, 1.0f},
    .backgroundColorB = {0.1f, 0.7f, 0.8f, 1.0f},
    .gradientAngle = 25.0f
});
```

### `size_t add(const Text& text)`
Convertit un texte en séries de rectangles de glyphes, ajoutés au batch.

```cpp
size_t glyphCount = renderer.add({
    .content = "Audio Engine",
    .font = "./assets/Inter-Regular.ttf",
    .color = {0, 0, 0, 1},
    .position = {120, 140},
    .fontSize = 42,
    .letterSpacing = 1.0f
});
```

### `void draw(const glm::mat4 projection)`
Sanitize les instances, upload SSBO, puis draw instancié et reset du batch.

```cpp
glm::mat4 projection = /* ortho */;
renderer.draw(projection);
```

### `void setBackgroundColor(const glm::vec4& color)`
Définit la couleur de clear utilisée à chaque `draw`.

```cpp
renderer.setBackgroundColor({1.0f, 1.0f, 1.0f, 1.0f});
```
