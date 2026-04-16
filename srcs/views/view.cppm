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

  virtual void addChild(std::shared_ptr<View> child) {
    if (child && child->parent == nullptr) {
      child->parent = this;
      children.emplace_back(child);
      markDirty();
    }
  }

  void removeChild(View *child) {
    if (!child) {
      return;
    }
    auto it =
        std::ranges::find_if(children, [&](const std::shared_ptr<View> &c) {
          return c.get() == child;
        });

    if (it != children.end()) {
      (*it)->parent = nullptr;
      children.erase(it);

      markDirty();
    }
  }

  [[nodiscard]] const std::vector<std::shared_ptr<View>> &
  getChildren() const noexcept {
    return children;
  }

  virtual void draw(Renderer &renderer) {

    layout();

    std::vector<std::shared_ptr<View>> sorted = children;
    std::stable_sort(
        sorted.begin(), sorted.end(),
        [](const auto &a, const auto &b) { return a->zIndex < b->zIndex; });

    // Dessine dans l'ordre
    for (auto &child : sorted) {
      if (child && child->isVisible()) {
        child->draw(renderer);
      }
    }
  }

  virtual void onMouseEvent(const MouseEventView & /*mouse*/) {}
  virtual void onKeyboardEvent(const KeyboardEventView & /*keyboard*/) {}

  virtual glm::vec2 getPosition() {
    update();
    return relativePosition;
  }

  virtual glm::vec2 getSize() {
    update();
    return glm::vec2{geometry.z, geometry.w};
  }

  void setPosition(const glm::vec2 &p) {
    relativePosition = p;

    markDirty();
  }

  glm::vec2 getOverflows() {
	return {};
  }

  // TODO sanitize data
  void setSize(const glm::vec2 &s) {
    geometry.z = s.x;
    geometry.w = s.y;
    markDirty();
  }

  void setVisible(bool v) { visible = v; }

  void setKeyboardFocus(bool v) { keyboardFocus = v; }

  void setMouseFocus(bool v) { mouseFocus = v; }

  const bool &isVisible() const { return visible; }

  virtual bool contain(const glm::vec2 &mouse) const {
    return mouse.x >= geometry.x && mouse.x <= geometry.x + geometry.z &&
           mouse.y >= geometry.y && mouse.y <= geometry.y + geometry.w;
  }

  const bool &isKeyboardFocused() const { return keyboardFocus; }
  const bool &isMouseFocused() const { return mouseFocus; }

  int zIndex = 0;

protected:
  virtual void layout() {
    const glm::vec2 apos = getAbsolutePosition();

    geometry.x = apos.x;
    geometry.y = apos.y;
  }

  // TODO make private
  glm::vec4 geometry = {0.0f, 0.0f, 0.0f, 0.0f};

  std::vector<std::shared_ptr<View>> children;

  virtual void markDirty() {
    if (updateDepth == 0)
      dirty = true;
  }

  const bool &isDirty() const { return dirty; }

  void update() {
    if (!dirty || updateDepth > 0)
      return;
    updateDepth++;
    layout();
    updateDepth--;
    dirty = false;
  }

  virtual glm::vec2 getAbsolutePosition() final {
    update();

    glm::vec2 pos(relativePosition);
    View *p = parent;
    while (p != nullptr) {
      pos += p->relativePosition;
      p = p->parent;
    }

    return pos;
  }

private:
  View *parent = nullptr;
  bool visible = true;
  bool keyboardFocus = false;
  bool mouseFocus = false;
  bool dirty = true;
  int updateDepth = 0;

  glm::vec2 relativePosition = {0.0f, 0.0f};
};

} // namespace mka::graphic
