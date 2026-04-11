module;

#include <algorithm>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
export module mka.graphic.view;
import mka.graphic.opengl.renderer;
import mka.graphic.keyboardview;
import mka.graphic.mouseview;

export namespace mka::graphic {

class View {
public:
  // Constructeur / Destructeur
  View() = default;
  virtual ~View() = default;

  const View *getParent() const { return parent; }

  // --- Gestion des enfants ---
  virtual View& addChild(std::unique_ptr<View> child) {
    child->parent = this;
    children.push_back(std::move(child));
	return *this;
  }

  virtual View& removeChild(View *child) {
    children.erase(std::remove_if(children.begin(), children.end(),
                                  [&](const std::unique_ptr<View> &c) {
                                    return c.get() == child;
                                  }),
                   children.end());
	return *this;
  }

  const std::vector<std::unique_ptr<View>> &getChildren() const {
    return children;
  }

  virtual void draw(Renderer &renderer) = 0;
  virtual void onMouseEvent(const MouseEventView & /*mouse*/) {}
  virtual void onKeyboardEvent(const KeyboardEventView & /*keyboard*/) {}

  glm::vec2 getPosition() const { return glm::vec2{geometry.x, geometry.y}; }
  virtual glm::vec2 getSize() { return glm::vec2{geometry.z, geometry.w}; }

  View &setPosition(const glm::vec2 &p) {
    geometry.x = p.x;
    geometry.y = p.y;
    return *this;
  }

  View &setSize(const glm::vec2 &s) {
    geometry.z = s.x;
    geometry.w = s.y;
    return *this;
  }

  View &setVisible(bool v) {
    visible = v;
    return *this;
  }

  View &setKeyboardFocus(bool v) {
    keyboardFocus = v;
    return *this;
  }

  View &setMouseFocus(bool v) {
    mouseFocus = v;
    return *this;
  }

  const bool &isVisible() const { return visible; }

  virtual bool contain(const glm::vec2 &mouse) const {
    return mouse.x >= geometry.x && mouse.x <= geometry.x + geometry.z &&
           mouse.y >= geometry.y && mouse.y <= geometry.y + geometry.w;
  }

  const bool &isKeyboardFocused() const { return keyboardFocus; }
  const bool &isMouseFocused() const { return mouseFocus; }

  int zIndex = 0;

protected:
  glm::vec4 geometry = {0.0f, 0.0f, 0.0f, 0.0f};

  std::vector<std::unique_ptr<View>> children;

private:
  View *parent = nullptr;
  bool visible = true;
  bool keyboardFocus = false;
  bool mouseFocus = false;
};

} // namespace mka::graphic
