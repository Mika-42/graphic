module;
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>

export module mka.graphic.view.floatview;
import mka.graphic.view;
import mka.graphic.opengl.renderer;

/*
 *        +------------------------------------------+
 *        |                         +---------------+|
 *        |                         |               ||
 *        |+--------------+         |        C      ||
 *        ||              |         +---------------+|
 *        ||      A       |+------------+            |
 *        ||              ||            |            |
 *        |+--------------+|     B      |            |
 *        |                |            |            |
 *        |                +------------+            |
 * (x, y) +------------------------------------------+
 */

export namespace mka::graphic {
/*
 * Place your children using relative position
 */
class FloatView : public View {
private:
  using View::setSize;

public:
  FloatView() : View() {}

  virtual void draw(Renderer & /*renderer*/) override { layout(); }

private:
  virtual void layout() override {

    if (children.empty()) {
      geometry.z = 0.0f;
      geometry.w = 0.0f;
      return;
    }

    float minX = 0.0f;
    float minY = 0.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;

    for (auto &child : children) {
      if (!child) {
        continue;
      }

      const glm::vec2 relPos = child->getRelativePosition();
      const glm::vec2 childSize = child->getSize();

      minX = glm::min(minX, relPos.x);
      minY = glm::min(minY, relPos.y);
      maxX = glm::max(maxX, relPos.x + childSize.x);
      maxY = glm::max(maxY, relPos.y + childSize.y);

      const glm::vec2 absPos = getAbsolutePosition() + relPos;
      child->setAbsolutePosition(absPos);
    }

    geometry.z = maxX - minX;
    geometry.w = maxY - minY;
  }
};

} // namespace mka::graphic
