# Module `mka.graphic.context`

[⬅ Retour index](./README.md) · [➡ Module window](./mka.graphic.window.md)

## Types
- `enum class API`
- `enum class Loader`
- `class Context`

## Fonctions
- `std::unique_ptr<Context> createContext(API api, Loader loader = Loader::None)`

## Méthodes principales de `Context`
- `bool init(GLFWwindow* window)`
- `std::string getName() const`
- `API getAPI() const`
- `void makeCurrent()`
- `void swapBuffers()`

## Exemple d'usage
```cpp
auto ctx = mka::graphic::createContext(
    mka::graphic::API::OpenGL,
    mka::graphic::Loader::Glad
);
if (!ctx) {
    // backend non disponible
}
```
