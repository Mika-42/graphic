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
		using View::addChild;
		using View::removeChild;
public:

  template<typename T>
  T& addChild(std::unique_ptr<T> child) {
	childRelatives[child.get()] = {0, 0};
    return View::addChild<T>(std::move(child));
  }

  virtual View& removeChild(View *child) override {
    childRelatives.erase(child);
    return View::removeChild(child);
  }

  virtual void draw(Renderer & /*renderer*/) override { layout(); }

private:
  virtual void layout() final {

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
		if (!child) continue;
		glm::vec2 relPos = childRelatives[child.get()];
      glm::vec2 childSize = child->getSize();
      
      minX = glm::min(minX, relPos.x);
      minY = glm::min(minY, relPos.y);
      maxX = glm::max(maxX, relPos.x + childSize.x);
      maxY = glm::max(maxY, relPos.y + childSize.y);
    }
	
	geometry.z = maxX - minX;
    geometry.w = maxY - minY;
	
	for (auto& child : children) {
      if (!child) continue;
      
      glm::vec2 relPos = childRelatives[child.get()];
      glm::vec2 absPos = getPosition() + relPos;
      child->setPosition(absPos);
    }
  }

  std::unordered_map<View *, glm::vec2> childRelatives;
};

} // namespace mka::graphic
