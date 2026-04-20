module;
#include <algorithm>
#include <glm/glm.hpp>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include "debug.hpp"

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

public: // parent/children management
  virtual const View *getParent() const final { return parent; }

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

  [[nodiscard]] const std::vector<std::shared_ptr<View>> &
  getChildren() const noexcept {
    return children;
  }

public: // getters
	
  enum class PositionType : uint8_t { Relative, Absolute };

  glm::vec2 getPosition(PositionType type = PositionType::Relative) {
    update();

	if(type == PositionType::Relative) {
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
    update();
    return glm::vec2{geometry.z, geometry.w};
  }
  const glm::vec2 &getOverflows() { 
	  computeOverflow();
	  return overflows; 
  }

  glm::vec4 getClipRadius() { return clipRect.radius; }
  const int32_t& getClipIndex() { return currentClipIndex; }
  const glm::vec4 &getGeometry() { return geometry; }
  const bool &isVisible() const { return visible; }
  const bool &isKeyboardFocused() const { return keyboardFocus; }
  const bool &isMouseFocused() const { return mouseFocus; }
  bool isClip() const { return clipRect.flags.y; }

public: //setters

  void setPosition(const glm::vec2 &p) {
    relativePosition = p;

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

  void setClip(bool enabled) {
	if(enabled) {
		clipRect.flags.y = CLIP;
	} else {
		clipRect.flags.y = 0.0f;
	}
	markDirty();
  }

  void setRadius(const glm::vec4& radius) {
	clipRect.radius = radius;
  }

  virtual void draw(Renderer &) {} //TODO make it virtual pure
protected:
 virtual void update(Renderer &renderer) {
    layout();

    computeOverflow();

	clipRect.geometry = geometry;
	currentClipIndex = parent ? parent->getClipIndex() : NO_CLIP;
	
	if(isClip()) {
		clipRect.flags.z = currentClipIndex;
		currentClipIndex = renderer.add(clipRect);
	}

	updateChild(renderer);

	draw(renderer);
 }

 void updateChild(Renderer &renderer) {
	std::vector<std::shared_ptr<View>> sorted = children;
    std::stable_sort(
        sorted.begin(), sorted.end(),
        [](const auto &a, const auto &b) { return a->zIndex < b->zIndex; });
 
	 for (auto &child : sorted) {	
		 if (child && child->isVisible()) {
			child->update(renderer);
		}
	}

 }

public:
  virtual void onMouseEvent(const MouseEventView & /*mouse*/) {}
  virtual void onKeyboardEvent(const KeyboardEventView & /*keyboard*/) {}

  virtual bool contain(const glm::vec2 &mouse) const {
    return mouse.x >= geometry.x && mouse.x <= geometry.x + geometry.z &&
           mouse.y >= geometry.y && mouse.y <= geometry.y + geometry.w;
  }

  int zIndex = 0;

protected:
  virtual void layout() {
    const glm::vec2 apos = getPosition(PositionType::Absolute);

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
  bool visible = true;
  bool keyboardFocus = false;
  bool mouseFocus = false;
  bool dirty = true;
  int updateDepth = 0;

  Rectangle clipRect;
  int32_t currentClipIndex = NO_CLIP;

  glm::vec2 relativePosition = glm::vec2(0.0f);
  glm::vec2 overflows = glm::vec2(0.0f);
};

} // namespace mka::graphic
