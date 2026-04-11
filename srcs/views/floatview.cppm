module;
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>
#include <limits>

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
		using View::addChild;
		using View::removeChild;
		using View::getSize;
public:

  virtual View& addChild(std::unique_ptr<View> child) override {
    childRelatives[child.get()] = child->getPosition();
    return View::addChild(std::move(child));
  }

  virtual View& removeChild(View *child) override {
    childRelatives.erase(child);
    return View::removeChild(child);
  }

  FloatView& move(View *child, const glm::vec2 &p) {
    if (childRelatives.contains(child)) {
      childRelatives[child] = p;
    }
    layout();
	return *this;
  }

  virtual glm::vec2 getSize() override {
	layout();
  	return View::getSize();
  }

  virtual void draw(Renderer & /*renderer*/) override { layout(); }

private:
  virtual void layout() final {

	  if (children.empty()) {
		geometry.z = 0.0f;
		geometry.w = 0.0f;
		return;
	  }

      float minX = std::numeric_limits<float>::max();
      float minY = std::numeric_limits<float>::max();
      float maxX = std::numeric_limits<float>::min();
      float maxY = std::numeric_limits<float>::min();

	  for (auto &child : children) {

      glm::vec2 relativePos = childRelatives[child.get()];

      glm::vec2 absolutePos = getPosition() + relativePos;
      child->setPosition(absolutePos);

      glm::vec2 size = child->getSize();
      minX = glm::min(minX, relativePos.x);
      minY = glm::min(minY, relativePos.y);
      maxX = glm::max(maxX, relativePos.x + size.x);
      maxY = glm::max(maxY, relativePos.y + size.y);
    }
	
	geometry.z = maxX - minX;
    geometry.w = maxY - minY;
	
  }

  std::unordered_map<View *, glm::vec2> childRelatives;
};

} // namespace mka::graphic
