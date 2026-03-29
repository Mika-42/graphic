# Documentation des modules `mka.graphic`

Cet index centralise la documentation par module avec:
- enums détaillés (toutes les valeurs),
- méthodes/fonctions expliquées,
- exemples d'utilisation couvrant les APIs principales.

## Navigation interactive
- [mka.graphic.context](./mka.graphic.context.md)
- [mka.graphic.window](./mka.graphic.window.md)
- [mka.graphic.sanitize](./mka.graphic.sanitize.md)
- [mka.graphic.opengl.uniform](./mka.graphic.opengl.uniform.md)
- [mka.graphic.opengl.shader](./mka.graphic.opengl.shader.md)
- [mka.graphic.opengl.renderer](./mka.graphic.opengl.renderer.md)

## Renvoi vers tous les exemples
- [Exemples module context](./mka.graphic.context.md#classe-context)
- [Exemples module window](./mka.graphic.window.md#m%C3%A9thodes)
- [Exemples module sanitize](./mka.graphic.sanitize.md#fonctions)
- [Exemples module uniform](./mka.graphic.opengl.uniform.md#exemples-usage-de-gluniform-avec-plusieurs-types)
- [Exemples module shader](./mka.graphic.opengl.shader.md#m%C3%A9thodes)
- [Exemples module renderer](./mka.graphic.opengl.renderer.md#classe-template-renderermax_rectangle_count)

## Code minimal à créer
Le squelette minimal ci-dessous crée un contexte OpenGL + une fenêtre + un renderer, puis lance la boucle.

```cpp
#include <memory>
#include <glm/glm.hpp>

import mka.graphic.window;
import mka.graphic.opengl.renderer;

class MinimalApp : public mka::graphic::Window {
public:
    MinimalApp(std::unique_ptr<mka::graphic::Context> ctx)
        : Window(800, 600, "Minimal", std::move(ctx)) {}

    void render() override {
        const glm::mat4 projection = getOrthographicProjection();
        renderer.setBackgroundColor({1, 1, 1, 1});
        renderer.add({
            .geometry = {80, 80, 240, 120},
            .backgroundColorA = {0.2f, 0.4f, 0.8f, 1.0f},
            .backgroundColorB = {0.1f, 0.8f, 0.7f, 1.0f},
            .gradientAngle = 20.0f
        });
        renderer.draw(projection);
    }

private:
    mka::graphic::gl::Renderer<1024> renderer;
};

int main() {
    auto ctx = mka::graphic::createContext(
        mka::graphic::API::OpenGL,
        mka::graphic::Loader::Glad
    );
    MinimalApp app(std::move(ctx));
    return app.run();
}
```
