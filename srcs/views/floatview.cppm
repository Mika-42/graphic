module;
#include <glm/glm.hpp>
#include <unordered_map>
#include <memory>

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
public:
  void setSize(const glm::vec2 &s) = delete;

  virtual void addChild(std::unique_ptr<View> child) {
    childRelatives[child.get()] = child->getPosition();
    View::addChild(std::move(child));
  }

  virtual void removeChild(View *child) override {
    childRelatives.erase(child);
    View::removeChild(child);
  }

  virtual void draw(Renderer & /*renderer*/) override { layout(); }

private:
  virtual void layout() final {

    for (auto &child : children) {
      
	  glm::vec2 relativePos = childRelatives[child.get()];
      
      glm::vec2 absolutePos = getPosition() + relativePos;
      child->setPosition(absolutePos);
    }
  }

  std::unordered_map<View *, glm::vec2> childRelatives;
};

} // namespace mka::graphic
