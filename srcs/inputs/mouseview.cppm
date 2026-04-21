
module;
#include <glm/glm.hpp>
export module mka.graphic.mouseview;
import mka.graphic.mouse;

export namespace mka::graphic {

struct MouseEventView {
public:
  MouseEventView(MouseEvent &ms) : m(ms) {}

  bool isPressed(MouseButton btn) const { return m.isPressed(btn); }

  bool isReleased(MouseButton btn) const { return m.isReleased(btn); }

  bool isScrollUp(MouseButton btn) const { return m.isScrollUp(btn); }

  bool isScrollDown(MouseButton btn) const { return m.isScrollDown(btn); }

  glm::vec2 scroll() const { return m.scroll; }
  glm::vec2 position() const { return m.position; }
  float cursorRadius() const { return m.cursorRadius; }

private:
  MouseEvent &m;
};

} // namespace mka::graphic
