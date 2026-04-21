/**
 * @file window.cppm
 * @brief C++20 module implementation for `window`.
 */
module;

#include "debug.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iterator>
#include <memory>
#include <random>
#include <vector>
#include <ranges>

export module mka.graphic.window;

export import mka.graphic.context;      // TODO check if usefull to export
export import mka.graphic.mouseview;    // TODO check if usefull to export
export import mka.graphic.keyboardview; // TODO check if usefull to export

import mka.graphic.keyboard;
import mka.graphic.mouse;
import mka.graphic.opengl.renderer;
import mka.graphic.view;

/// @brief Keep OpenGL viewport in sync with framebuffer size.
void framebuffer_size_callback(GLFWwindow * /*window*/, int width, int height) {
  glViewport(0, 0, width, height);
}

namespace mka::graphic {
class RootView final: private View {
public: // expose base method
	  using View::getSize;
	  using View::getOverflows; 
	  using View::isKeyboardFocused;
	using View::isMouseFocused;

public:

  RootView() : View() {}

    void addRoot(std::shared_ptr<View> child) {
		if (child && children.empty()) {
			View::addChild(child);
		}
	}
//--- NOTE : everything under, (public/private/protected) do not exist for child trying to access by parent
	void updateRoot(Renderer &renderer) {
		updateChild(renderer);
	}

	void setRootSize(const glm::vec2& g) {
		setPosition(glm::vec2(0.0f));
		setSize(g);
	}

  // Nouvelle méthode publique : trie TOUS les enfants visibles
  void topoZSort() noexcept {
    sortedViews.clear();

    for (auto& child : getChildren()) {
      if (child && child->isVisible()) {
        topoZSortRecursive(child.get());
      }
    }
  }

  std::vector<View*>& getHoveredViews(const MouseEventView& mouse) noexcept {
	  hovered.clear();
	  std::function<void(View*)> collect = [&](View* node) noexcept {
		if (!node) return;
	
		for (auto& child : node->getChildren()) {
		  
		  if (!child || !child->isVisible()) {
			  continue;
		  }
		  
		  if (child->isClip() && !child->clipContain(mouse)) {
			  continue;
		  }
		  
		  if (child->contain(mouse)) {
			hovered.push_back(child.get());
			collect(child.get());
		  }
		}
	  };
	  
	  collect(children[0].get());
	  return hovered;  // Top-most = hovered.back() !
	}

private:
  void draw(Renderer &) {}

  // Méthode récursive privée
  void topoZSortRecursive(View* view) noexcept {
    if (!view || !view->isVisible()) return;
    
    // Trier les enfants par zIndex
    auto& ch = view->getChildren();
    
    // Parcours récursif des enfants
    for (auto& child : ch) {
      topoZSortRecursive(child.get());
    }
    
    // Ajouter le noeud courant (post-order)
    sortedViews.push_back(view);
  }

public:
  std::vector<View*> sortedViews;  // public pour accès externe si besoin
  std::vector<View*> hovered;

};
} // namespace mka::graphic

export namespace mka::graphic {

/// @brief High-level runtime state of a window instance.
enum class State { Inited, Running, Stopped, Terminated };

struct Time {
  double now = 0.0;   /// Absolute time from GLFW (milliseconds).
  double delta = 0.0; /// Delta between two frames (milliseconds).
};

/**
 * @brief Base window abstraction hosting the render loop.
 *
 * Derived classes only implement `render()` while this class manages:
 * - GLFW init/teardown
 * - API-specific window hints
 * - Context setup and buffer presentation
 * - Projection updates on resize
 */
class Window {
public:
  Window(size_t width, size_t height, const std::string &name,
         std::unique_ptr<Context> ctx)
      : size(width, height), name(name), ctx(std::move(ctx)), window(nullptr),
        mouseEventView(mouseEvent), keyboardEventView(keyboardEvent) {
    if (!glfwInit()) {
      state = State::Terminated;
      return;
    }

    state = State::Inited;

    switch (this->ctx->getAPI()) {
    case API::OpenGL:
      glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      break;

    case API::Vulkan:
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      break;

    case API::None:

    default:
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      break;
    }

    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    window =
        glfwCreateWindow(size.x, size.y, this->name.c_str(), nullptr, nullptr);

    if (!window) {
      state = State::Terminated;
      glfwTerminate();
      return;
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetWindowSizeCallback(window, [](GLFWwindow *win, int w, int h) {
      Window *self = static_cast<Window *>(glfwGetWindowUserPointer(win));
      self->orthographicProjection =
          glm::ortho(0.0f, float(w), float(h), 0.0f, -1.0f, 1.0f);
      self->size = glm::vec2(w, h);
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow *win, double x, double y) {
      Window *self = static_cast<Window *>(glfwGetWindowUserPointer(win));
      self->mouseEvent.position = glm::vec2(x, y);
    });

    glfwSetScrollCallback(
        window, [](GLFWwindow *win, double xOffset, double yOffset) {
          Window *self = static_cast<Window *>(glfwGetWindowUserPointer(win));
          self->mouseEvent.scroll = glm::vec2(xOffset, yOffset);

          if (yOffset > 0)
            self->mouseEvent.set(MouseButton::Middle, MouseState::ScrollUp);
          if (yOffset < 0)
            self->mouseEvent.set(MouseButton::Middle, MouseState::ScrollDown);
        });

    glfwSetMouseButtonCallback(
        window, [](GLFWwindow *win, int button, int action, int /*mods*/) {
          Window *self = static_cast<Window *>(glfwGetWindowUserPointer(win));

          MouseState state = (action == GLFW_PRESS) ? MouseState::Pressed
                                                    : MouseState::Released;
          MouseButton btn;

          switch (button) {
          case GLFW_MOUSE_BUTTON_LEFT:
            btn = MouseButton::Left;
            break;
          case GLFW_MOUSE_BUTTON_MIDDLE:
            btn = MouseButton::Middle;
            break;
          case GLFW_MOUSE_BUTTON_RIGHT:
            btn = MouseButton::Right;
            break;
          default:
            return;
          }

          self->mouseEvent.set(btn, state);
        });

    if (!this->ctx || !this->ctx->init(window)) {
      state = State::Terminated;
      return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    renderer = std::make_unique<Renderer>();
  }

  /// @brief Run the blocking frame loop until close.
  int run() {
    if (state != State::Inited)
      return -1;

    state = State::Running;
    lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();

      handleKeyboard();

      double current = glfwGetTime();
      time.delta = (current - lastTime) * 1000.0;
      time.now = current * 1000.0;
      lastTime = current;

      ctx->makeCurrent();

      render(mouseEventView, keyboardEventView, time);

      ctx->swapBuffers();
    }

    state = State::Stopped;
    return 0;
  }

  ~Window() {
    if (window) {
      glfwDestroyWindow(window);
    }
    glfwTerminate();
  }

  const State &getState() const { return state; }

  void setBackgroundColor(const glm::vec4 &color) { bgColor = color; }

  void setRoot(std::shared_ptr<View> root) {
	if(!root) { return; }
    rootView = std::make_shared<RootView>(); // TODO check if .reset() should be better
    rootView->addRoot(root);
  }

private:
  void render(const MouseEventView &mouse, const KeyboardEventView &keyboard,
              const Time & /*time*/) {

    if (!renderer) {
      DEBUG_LOG("renderer is not initialized.");
      return;
    }
	
    renderer->setBackgroundColor(bgColor);

    if (!rootView) {
      DEBUG_LOG("root view is not set.");
      return;
    }

    rootView->setRootSize(size);

    rootView->topoZSort();

    for (auto &view : rootView->sortedViews) {
      if (!view) {
        continue;
      }

	  if (view->isKeyboardFocused()) {
        view->onKeyboardEvent(keyboard);
      }
	}
	
	rootView->updateRoot(*renderer);

	handleMouse(mouse);

    renderer->draw(orthographicProjection);
  }

  void handleMouse(const MouseEventView& mouse) noexcept {
	auto& hits = rootView->getHoveredViews(mouse);
	View* prevTop = focusedView;

	if(!hits.empty()) {
		View* newTop = hits.back();

		if (newTop != prevTop) {
			if (prevTop) { 
				prevTop->onMouseLeave(mouse);
			}

			focusedView = newTop;
			newTop->onMouseEnter(mouse);
		} 

		newTop->onMouseMove(mouse);

	} else if (prevTop) {
		prevTop->onMouseLeave(mouse);
		focusedView = nullptr;
	}
  }

private:
  glm::vec2 size = {0.0, 0.0};
  glm::mat4 orthographicProjection = glm::mat4(1.0f);
  glm::vec4 bgColor = glm::vec4(1.0f);
  std::string name;
  std::unique_ptr<Context> ctx = nullptr;
  State state = State::Terminated;
  GLFWwindow *window = nullptr;
  MouseEvent mouseEvent;
  KeyboardEvent keyboardEvent;

  MouseEventView mouseEventView;
  KeyboardEventView keyboardEventView;

  double lastTime = 0.0;
  Time time;

  std::unique_ptr<Renderer> renderer;
  std::shared_ptr<RootView> rootView = nullptr;

  View* focusedView = nullptr;
private:
  void handleKeyboard() {
    for (auto &i : keyboardEvent.castTable) {
      i.state = glfwGetKey(window, i.glfw) == GLFW_RELEASE ? KeyState::Released
                                                           : KeyState::Pressed;
    }
  }
};
} // namespace mka::graphic
