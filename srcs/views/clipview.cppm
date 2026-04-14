module;
#include <glm/glm.hpp>

export module mka.graphic.view.clipview;
import mka.graphic.view;
import mka.graphic.opengl.rectangle;
import mka.graphic.opengl.renderer;

export namespace mka::graphic {

class ClipView : public View {
public:
  ClipView() : View() {}

  void setRadius(const glm::vec4 &r) {
    radius = r;
    markDirty();
    return ;
  }

  const glm::vec4 &getRadius() {
    update();
    return radius;
  }

  void draw(Renderer &renderer) override {
    layout();
    viewport.geometry = geometry;
    viewport.radius = radius;
    viewport.flags = CLIP_VIEW;
    renderer.add(viewport);

    for (auto &child : children) {
      if (child && child->isVisible()) {
        child->draw(renderer);
      }
    }
  }

private:
  void layout() override {
    for (auto &child : children) {

      if (!child) {
        continue;
      }
      const glm::vec2 relPos = child->getRelativePosition();
      const glm::vec2 absPos = getAbsolutePosition() + relPos;
      child->setAbsolutePosition(absPos);
    }
  }

private:
  Rectangle viewport{};
  glm::vec4 radius = glm::vec4(0.0f);
};
} // namespace mka::graphic
