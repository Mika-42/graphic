module;

#include <algorithm>
#include <glm/glm.hpp>
#include <memory>
#include <ranges>
#include <vector>

export module mka.graphic.view.stackview;
import mka.graphic.view;
import mka.graphic.sanitize;
import mka.graphic.opengl.renderer;

/*
 * +-------------+
 * |     C       |
 * +-------------+
 * |     B       |
 * +-------------+
 * |     A       |
 * +-------------+
 *
 * reverse: 
 *
 * +-------------+
 * |     A       |
 * +-------------+
 * |     B       |
 * +-------------+
 * |     C       |
 * +-------------+
 *
 * direction: 
 * +-------------+-------------+-------------+
 * |     A       |      B      |      C      |
 * +-------------+-------------+-------------+
 *
 * reverse-direction: 
 * +-------------+-------------+-------------+
 * |     C       |      B      |      A      |
 * +-------------+-------------+-------------+
 *
 * align-left
 *
 * +--------+
 * |     C  |
 * +--------+----+
 * |     B       |
 * +-------------+----+
 * |     A            |
 * +------------------+
 *
 * align-right
 *
 *           +--------+
 *           |     C  |
 *      +----+--------+
 *      |          B  |
 * +----+-------------+
 * |               A  |
 * +------------------+
 *
 *
 * align-center
 *
 *      +-------+
 *      |  B    |  
 * +----+-------+----+
 * |       A         |
 * +-----------------+
 * 
 * gap: 
 * +-------------+
 * |     C       |
 * +-------------+
 *	//////////////
 * +-------------+
 * |     B       |
 * +-------------+
 *	//////////////
 * +-------------+
 * |     A       |
 * +-------------+
 * */

export namespace mka::graphic {

enum class Align { Left, Right, Center, Top, Bottom };

enum class Orientation { Vertical, Horizontal };

class StackView : public View {
public:
  void setSize(const glm::vec2 &s) = delete;

  virtual void draw(Renderer& /*renderer*/) override {
	layout();
  }

  glm::vec2 getSize() override {
	layout();
	return View::getSize();
  }


  
  void setGap(float v) { gap = sanitizeFloat(v, 0.0f); }

  void setAlign(Align a) { align = a; }

  void setOrientation(Orientation o) { orientation = o; }

  void reverse(bool r) { reverseOrder = r; }

  const float &getGap() { return gap; }
  const Align &getAlign() { return align; }
  const Orientation &getOrientation() { return orientation; }
  const bool &isReversed() { return reverseOrder; }

private:
  float alignOffset(const std::unique_ptr<View> &view) const {

    const float dw = geometry.z - view->getSize().x;
    const float dh = geometry.w - view->getSize().y;
    const bool isVertical = (orientation == Orientation::Vertical);

    switch (align) {

    case Align::Right:
      return dw;
    case Align::Bottom:
      return dh;
    case Align::Center:
      return isVertical ? 0.5f * dw : 0.5f * dh;
    case Align::Left:
      return 0.0f;
    case Align::Top:
      return 0.0f;

    default:
      return 0.0f;
    }
  }

  virtual void vlayout() final {

    float offset = 0.0f;

    auto process = [&](std::unique_ptr<View> &child) {
      geometry.z = glm::max(geometry.z, child->getSize().x);

      child->setPosition(
          {alignOffset(child) + geometry.x, geometry.y + offset});
      offset += child->getSize().y + gap;
    };

    if (reverseOrder) {
      std::ranges::for_each(children | std::views::reverse, process);
    } else {
      std::ranges::for_each(children, process);
    }

    geometry.w = offset;
  }

  virtual void hlayout() final {

    float offset = 0.0f;

    auto process = [&](std::unique_ptr<View> &child) {
      geometry.w = glm::max(geometry.w, child->getSize().y);

      child->setPosition(
          {geometry.x + offset, geometry.y + alignOffset(child)});
      offset += child->getSize().x + gap;
    };

    if (reverseOrder) {
      std::ranges::for_each(children | std::views::reverse, process);
    } else {
      std::ranges::for_each(children, process);
    }

    geometry.z = offset;
  }

  virtual void layout() final {

    switch (orientation) {
    case Orientation::Vertical:
      vlayout();
      break;
    case Orientation::Horizontal:
      hlayout();
      break;
    default:
      vlayout();
    }
  }
private:
  Align align = Align::Left;
  Orientation orientation = Orientation::Vertical;
  float gap = 0.0f;
  bool reverseOrder = false;
};

} // namespace mka::graphic
