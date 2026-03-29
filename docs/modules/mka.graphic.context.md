# Module `mka.graphic.context`

[⬅ Retour index](./README.md) · [➡ Module window](./mka.graphic.window.md)

Ce module définit l’abstraction de **contexte graphique** (backend + loader) utilisée par la fenêtre pour initialiser et piloter le rendu.

## Enums

### `API`
- `API::None` : aucun backend.
- `API::OpenGL` : backend OpenGL.
- `API::Vulkan` : backend Vulkan (placeholder actuellement).
- `API::DirectX` : backend DirectX (placeholder actuellement).
- `API::Metal` : backend Metal (placeholder actuellement).

### `Loader`
- `Loader::None` : aucun loader OpenGL.
- `Loader::Glad` : loader GLAD.
- `Loader::Glew` : loader GLEW (placeholder actuellement).

## Classe `Context`

Interface de base pour tous les contextes graphiques.

### Méthodes

#### `bool init(GLFWwindow* window)`
Initialise le backend sur une fenêtre GLFW native.

```cpp
if (!ctx->init(glfwWindowPtr)) {
    // échec d'initialisation du backend
}
```

#### `std::string getName() const`
Renvoie un nom lisible du backend actif (utile pour logs/debug).

```cpp
std::println("Backend: {}", ctx->getName());
```

#### `API getAPI() const`
Renvoie la famille d’API utilisée par le contexte.

```cpp
if (ctx->getAPI() == mka::graphic::API::OpenGL) {
    // config OpenGL
}
```

#### `void makeCurrent()`
Active le contexte sur le thread courant avant le rendu.

```cpp
ctx->makeCurrent();
```

#### `void swapBuffers()`
Présente l’image rendue (swap front/back buffer).

```cpp
ctx->swapBuffers();
```

## Fonction exportée

### `std::unique_ptr<Context> createContext(API api, Loader loader = Loader::None)`
Fabrique une implémentation concrète de `Context` selon l’API et le loader choisis.

```cpp
auto ctx = mka::graphic::createContext(
    mka::graphic::API::OpenGL,
    mka::graphic::Loader::Glad
);
if (!ctx) {
    // combinaison API/loader invalide ou non implémentée
}
```
