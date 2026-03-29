# Module `mka.graphic.window`

[⬅ Retour index](./README.md) · [⬅ Module context](./mka.graphic.context.md) · [➡ Module renderer](./mka.graphic.opengl.renderer.md)

Ce module expose l’abstraction de **fenêtre applicative** et la boucle de rendu principale.

## Enum

### `State`
- `State::Inited` : fenêtre/context initialisés.
- `State::Running` : boucle de rendu active.
- `State::Stopped` : boucle terminée proprement.
- `State::Terminated` : erreur d’initialisation ou fermeture complète.

## Classe `Window`

Classe de base à hériter : l’application implémente `render()` et utilise `run()` pour exécuter la boucle.

### Méthodes

#### `Window(size_t width, size_t height, const std::string& name, std::unique_ptr<Context> ctx)`
Construit la fenêtre, applique les hints API, crée la fenêtre GLFW et initialise le contexte.

```cpp
class App : public mka::graphic::Window {
public:
    App(std::unique_ptr<mka::graphic::Context> ctx)
        : Window(1280, 720, "App", std::move(ctx)) {}

    void render() override {}
};
```

#### `virtual void render() = 0`
Callback utilisateur exécuté à chaque frame.

```cpp
void render() override {
    // ici: add/draw via renderer
}
```

#### `const glm::mat4& getOrthographicProjection() const`
Renvoie la projection orthographique tenue à jour au redimensionnement de la fenêtre.

```cpp
const glm::mat4 projection = getOrthographicProjection();
```

#### `int run()`
Démarre la boucle: `makeCurrent() -> render() -> swapBuffers() -> poll events`.

```cpp
int code = app.run();
if (code != 0) {
    // init incorrecte
}
```

#### `const State& getState()`
Expose l’état courant de la fenêtre/boucle.

```cpp
if (app.getState() == mka::graphic::State::Running) {
    // monitoring runtime
}
```

#### `const glm::vec2 getSize()`
Renvoie la taille courante de la fenêtre (mise à jour sur callback resize).

```cpp
auto size = app.getSize();
std::println("{}x{}", size.x, size.y);
```
