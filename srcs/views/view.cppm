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

template<typename Derived>
class View {
public:
  // Constructeur / Destructeur
  View() = default;

  virtual ~View() = default;

  const View *getParent() const { return parent; }

  // --- Gestion des enfants ---
	
  Derived& addChild(std::shared_ptr<View> child) {
	  if(child) {
		  child->parent = this;
		children.emplace_back(child);
	  markDirty();
	  }
	  return self();
  }

  Derived& removeChild(View *child) {
	  if(!child) {
		  return self(); 
	  }

    children.erase(std::remove_if(children.begin(), children.end(),
                                  [&](const std::shared_ptr<View> &c) {
                                    return c.get() == child;
                                  }),
                   children.end());
	markDirty();
	return self();
  }

  [[nodiscard]] const std::vector<std::shared_ptr<View>> &getChildren() const noexcept {
    return children;
  }

  virtual void draw(Renderer &renderer) = 0;
  virtual void onMouseEvent(const MouseEventView & /*mouse*/) {}
  virtual void onKeyboardEvent(const KeyboardEventView & /*keyboard*/) {}

  glm::vec2 getAbsolutePosition() { 
	  update();
	  return glm::vec2{geometry.x, geometry.y}; 
  }

  glm::vec2 getRelativePosition() { 
	update();
	  return relativePosition; 
  } 

  glm::vec2 getSize() {
	  update();
	  return glm::vec2{geometry.z, geometry.w}; 
  }

  Derived &setAbsolutePosition(const glm::vec2 &p) {
    geometry.x = p.x;
    geometry.y = p.y;
	markDirty();
    return self();
  }

  Derived &setRelativePosition(const glm::vec2 &p) {
	relativePosition = p;
	markDirty();
    return self();
  }

  Derived &setSize(const glm::vec2 &s) {
    geometry.z = s.x;
    geometry.w = s.y;
	markDirty();
    return self();
  }

  Derived &setVisible(bool v) {
    visible = v;
    return self();
  }

  Derived &setKeyboardFocus(bool v) {
    keyboardFocus = v;
    return self();
  }

  Derived &setMouseFocus(bool v) {
    mouseFocus = v;
    return self();
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
  virtual void layout() {}

  glm::vec4 geometry = {0.0f, 0.0f, 0.0f, 0.0f};

  glm::vec2 relativePosition = {0.0f, 0.0f};

  std::vector<std::shared_ptr<View>> children;

  virtual void markDirty() { 
    if (updateDepth == 0) dirty = true;
  }
  
  void update() {
    if (!dirty || updateDepth > 0) return;
    updateDepth++;
    layout();
    updateDepth--;
    dirty = false;
  }

  Derived& self() { return static_cast<Derived&>(*this); }
  const Derived& self() const { return static_cast<const Derived&>(*this); }

private:
  View *parent = nullptr;
  bool visible = true;
  bool keyboardFocus = false;
  bool mouseFocus = false;
  bool dirty = true;
  int updateDepth = 0;
};

} // namespace mka::graphic
