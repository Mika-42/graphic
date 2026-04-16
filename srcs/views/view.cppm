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
  
  // TODO sanitize data
  void setClipGeometry(const glm::vec4& geo) {
    clipGeometry = geo;
    markDirty();
  }

  // TODO sanitize data
  void setClipRadius(const glm::vec4& rad) {
    clipRadius = rad;
    markDirty();
  }

  // 🔥 Logique automatique : clip si geometry valide
  bool shouldClip() const {
    return clipGeometry.z > 0.0f && clipGeometry.w > 0.0f;
  }

  bool isClipActive() const { 
    return shouldClip(); 
  }

  const glm::vec4& getClipGeometry() const { 
    return clipGeometry; 
  }
  const glm::vec4& getClipRadius() const { 
    return clipRadius; 
  }

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
    auto it = std::ranges::find_if(children,
        [&](const std::shared_ptr<View> &c) { return c.get() == child; });

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
    std::stable_sort(sorted.begin(), sorted.end(),
        [](const auto& a, const auto& b) {
            return a->zIndex < b->zIndex;
        });
    
    // Dessine dans l'ordre
    for (auto& child : sorted) {
        if (child && child->isVisible()) {
            child->draw(renderer);
        }
    }
	  if(shouldClip()) {
		renderer.pushClip(clipGeometry, clipRadius);
	  }
  }

  virtual void onMouseEvent(const MouseEventView & /*mouse*/) {}
  virtual void onKeyboardEvent(const KeyboardEventView & /*keyboard*/) {}

  virtual glm::vec2 getAbsolutePosition() {
    update();
    return glm::vec2{geometry.x, geometry.y};
  }

  virtual glm::vec2 getRelativePosition() {
    update();
    return relativePosition;
  }

  virtual glm::vec2 getSize() {
    update();
    return glm::vec2{geometry.z, geometry.w};
  }

  // TODO sanitize data
  void setAbsolutePosition(const glm::vec2 &p) {
    geometry.x = p.x;
    geometry.y = p.y;
    markDirty();
  }

  // TODO sanitize data
  void setRelativePosition(const glm::vec2 &p) {
    relativePosition = p;
    markDirty();
  }

  // TODO sanitize data
  void setGeometry(const glm::vec4& g) {
    geometry = g;
    markDirty();
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
  virtual void layout() {}

  // TODO make private
  glm::vec4 geometry = {0.0f, 0.0f, 0.0f, 0.0f};


  // TODO make private
  glm::vec2 relativePosition = {0.0f, 0.0f};

  std::vector<std::shared_ptr<View>> children;

  virtual void markDirty() {
    if (updateDepth == 0)
      dirty = true;
  }

  const bool& isDirty() const { return dirty; }

  void update() {
    if (!dirty || updateDepth > 0)
      return;
    updateDepth++;
    layout();
    updateDepth--;
    dirty = false;
  }

private:
  View *parent = nullptr;
  glm::vec4 clipGeometry = glm::vec4(0.0f); 
  glm::vec4 clipRadius = glm::vec4(0.0f); 
  bool visible = true;
  bool keyboardFocus = false;
  bool mouseFocus = false;
  bool dirty = true;
  int updateDepth = 0;
};

} // namespace mka::graphic
