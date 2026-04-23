module;
#include "debug.hpp"
#include <algorithm>
#include <glm/glm.hpp>
#include <limits>
#include <memory>
#include <ranges>
#include <string>
#include <vector>

export module mka.graphic.view;
import mka.graphic.opengl.renderer;
import mka.graphic.sanitize;
import mka.graphic.keyboardview;
import mka.graphic.mouseview;
import mka.graphic.event;

export namespace mka::graphic {
class View {
public:
  // Constructeur / Destructeur
  View() = default;
  virtual ~View() = default;

public: // parent/children management
  [[nodiscard]] virtual const View *getParent() const noexcept final {
    return parent;
  }

  virtual void addChild(std::shared_ptr<View> child) {
    if (child && child->parent == nullptr) {
      child->parent = this;
      children.emplace_back(child);
      markDirty();
    }
  }

  virtual void removeChild(View *child) {
    if (!child) {
      return;
    }
    auto it =
        std::ranges::find_if(children, [&](const std::shared_ptr<View> &c) {
          return c.get() == child;
        });

    if (it != children.end()) {
      (*it)->parent = nullptr;
      (*it)->clipRect.flags.z = NO_CLIP;
      children.erase(it);
      markDirty();
    }
  }

  [[nodiscard]] virtual const std::vector<std::shared_ptr<View>> &
  getChildren() const noexcept final {
    return children;
  }

public: // getters
  enum class PositionType : uint8_t { Relative, Absolute };

  glm::vec2 getPosition(PositionType type = PositionType::Relative) {
    updateData();

    if (type == PositionType::Relative) {
      return relativePosition;
    }

    glm::vec2 pos(relativePosition);
    View *p = parent;
    while (p != nullptr) {
      pos += p->relativePosition;
      p = p->parent;
    }

    return pos;
  }

  glm::vec2 getSize() {
    updateData();
    return glm::vec2{geometry.z, geometry.w};
  }

  const glm::vec2 &getOverflows() {
    computeOverflow();
    return overflows;
  }

  const glm::vec2& getScroll() const { return scrollOffset; }
  glm::vec4 getClipRadius() const { return clipRect.radius; }
  const int32_t &getClipIndex() const { return currentClipIndex; }
  const glm::vec4 &getGeometry() const { return geometry; }
  const bool &isVisible() const { return visible; }
  const bool &isKeyboardFocused() const { return keyboardFocus; }
  const bool &isMouseFocused() const { return mouseFocus; }
  bool isClip() const { return clipRect.flags.y; }

public: // setters
  void setPosition(const glm::vec2 &p) {
    relativePosition = p;
    markDirty();
  }

  void setSize(const glm::vec2 &s) {
    geometry.z = sanitizeFloat(s.x);
    geometry.w = sanitizeFloat(s.y);
    markDirty();
  }

  void setVisible(bool v) { visible = v; }

  void setKeyboardFocus(bool v) { keyboardFocus = v; }

  void setClip(bool enabled) {
    clipRect.flags.y = enabled ? CLIP : 0.0f;
    markDirty();
  }

  void setRadius(const glm::vec4 &radius) { clipRect.radius = radius; }

  void setScroll(const glm::vec2& s) { scrollOffset = s; }
  void setScrollX(float x) { scrollOffset.x = x; }
  void setScrollY(float y) { scrollOffset.y = y; }

  virtual void draw(Renderer &) {} // TODO make it virtual pure

protected: // updates
  virtual void update(Renderer &renderer) {
    layout();

    computeOverflow();

    currentClipIndex = parent ? parent->getClipIndex() : NO_CLIP;

    if (isClip()) {
      clipRect.flags.z = currentClipIndex;
      currentClipIndex = renderer.add(clipRect);
    }

    updateChild(renderer);

    draw(renderer);
  }

  std::vector<std::shared_ptr<View>> getSortedChildren() const {
    auto sorted = children;
    std::ranges::stable_sort(sorted, {}, &View::zIndex);
    return sorted;
  }

  void updateChild(Renderer &renderer) {

    for (auto &child : getSortedChildren()) {
      if (child && child->isVisible()) {
        child->update(renderer);
      }
    }
  }

public:
  virtual void onMouseEnter(const MouseEventView & /*mouse*/) { 
	  mouseFocus = true; 
	  event::send(this, event::mouse::enter);
  }
  virtual void onMouseLeave(const MouseEventView & /*mouse*/) { 
	  mouseFocus = false; 
      event::send(this, event::mouse::leave);
  }
  virtual void onMouseMove(const MouseEventView & /*mouse*/) {
      event::send(this, event::mouse::move);
  }

  virtual void onKeyboardEvent(const KeyboardEventView & /*keyboard*/) {}

  virtual bool contain(const MouseEventView& mouse) {
    return clipContain(mouse);
  }

  virtual bool clipContain(const MouseEventView& mouse) const final {
	return distance(clipRect, mouse.position()) <= mouse.cursorRadius();
  }

  int zIndex = 0;

protected:
  virtual void layout() {
    const glm::vec2 apos = getPosition(PositionType::Absolute);
    geometry.x = apos.x + scrollOffset.x;
    geometry.y = apos.y + scrollOffset.y;
    clipRect.geometry = geometry;

  }

  std::vector<std::shared_ptr<View>> children;

  virtual void markDirty() {
    if (updateDepth == 0)
      dirty = true;
  }

  const bool &isDirty() const { return dirty; }

  void updateData() {
    if (!dirty || updateDepth > 0)
      return;
    updateDepth++;
    layout();
    updateDepth--;
    dirty = false;
  }

private:
  void computeOverflow() {
    if (children.empty() || geometry.z <= 0 || geometry.w <= 0) {
      overflows = glm::vec2(0.0f);
      return;
    }

    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();

    for (const auto &child : children) {
      if (child->geometry.z <= 0 || child->geometry.w <= 0)
        continue;

      minX = glm::min(minX, child->geometry.x);
      minY = glm::min(minY, child->geometry.y);
      maxX = glm::max(maxX, child->geometry.x + child->geometry.z);
      maxY = glm::max(maxY, child->geometry.y + child->geometry.w);
    }

    if (maxX <= minX) {
      overflows = glm::vec2(0.0f);
      return;
    }

    const float contRight = geometry.x + geometry.z;
    const float contBottom = geometry.y + geometry.w;

    // Overflow total (gauche + droite, haut + bas)
    overflows = {
        std::max(0.0f, minX - geometry.x) + std::max(0.0f, maxX - contRight),
        std::max(0.0f, minY - geometry.y) + std::max(0.0f, maxY - contBottom)};
  }

private:
  View *parent = nullptr;
  Rectangle clipRect = {};
  
  bool visible			= true;
  bool keyboardFocus	= false;
  bool mouseFocus		= false;
  bool dirty			= true;

  int32_t updateDepth		= 0;
  int32_t currentClipIndex	= NO_CLIP;

  glm::vec4 geometry			= glm::vec4(0.0f);
  glm::vec2 relativePosition	= glm::vec2(0.0f);
  glm::vec2 overflows			= glm::vec2(0.0f);
  glm::vec2 scrollOffset		= glm::vec2(0.0f);
};

} // namespace mka::graphic
