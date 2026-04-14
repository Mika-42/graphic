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
#include <ranges>

export module mka.graphic.window;

export import mka.graphic.context;   // TODO check if usefull to export
export import mka.graphic.mouseview; // TODO check if usefull to export
export import mka.graphic.keyboardview;

import mka.graphic.keyboard;
import mka.graphic.mouse;
import mka.graphic.opengl.renderer;
import mka.graphic.view;

/// @brief Keep OpenGL viewport in sync with framebuffer size.
void framebuffer_size_callback(GLFWwindow * /*window*/, int width, int height) {
  glViewport(0, 0, width, height);
}

namespace mka::graphic {
class RootView : public View {
private:
	using View::addChild;

public:

  RootView() : View() {}

  void draw(Renderer & /*renderer*/) override {}

    void addChild(std::shared_ptr<View> child) override {
		if (child) {
		  children.emplace_back(child);
		  markDirty();
		}
	}

  // Nouvelle méthode publique : trie TOUS les enfants visibles
  void topoZSort() noexcept {
    sortedViews.clear();
    for (auto& child : children) {
      if (child && child->isVisible()) {
        topoZSortRecursive(child.get());
      }
    }
  }

private:
  // Méthode récursive privée
  void topoZSortRecursive(View* view) noexcept {
    if (!view || !view->isVisible()) return;
    
    // Trier les enfants par zIndex
    auto ch = view->getChildren();
    std::stable_sort(ch.begin(), ch.end(), [](auto a, auto b) { return a->zIndex < b->zIndex; });
    
    // Parcours récursif des enfants
    for (auto child : ch) {
      topoZSortRecursive(child.get());
    }
    
    // Ajouter le noeud courant (post-order)
    sortedViews.push_back(view);
  }

public:
  std::vector<View*> sortedViews;  // public pour accès externe si besoin
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
    rootView->addChild(root);
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

    rootView->setAbsolutePosition({0.0f, 0.0f});
    rootView->setSize(size);

    rootView->topoZSort();

    for (auto &view : rootView->sortedViews | std::views::reverse) {
      if (!view) {
        continue;
      }
      if (view->contain(mouse.position())) {
        view->setMouseFocus(true);
        view->onMouseEvent(mouse);
        break;
      }
    }

    for (auto &view : rootView->sortedViews) {
      if (!view) {
        continue;
      }
      if (view->isKeyboardFocused()) {
        view->onKeyboardEvent(keyboard);
      }

      if (view->isVisible()) {
        view->draw(*renderer);
      }
    }

    for (auto &view : rootView->sortedViews) {
      if (!view) {
        continue;
      }
      view->setMouseFocus(false);
    }

    renderer->draw(orthographicProjection);
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

private:
  void handleKeyboard() {
    for (auto &i : keyboardEvent.castTable) {
      i.state = glfwGetKey(window, i.glfw) == GLFW_RELEASE ? KeyState::Released
                                                           : KeyState::Pressed;
    }
  }
};
} // namespace mka::graphic
