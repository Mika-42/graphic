module;

#include <GLFW/glfw3.h>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string>
#include <vector>

export module mka.graphic.window;
export import mka.graphic.context;
export import mka.graphic.mouse;

/// @brief Keep OpenGL viewport in sync with framebuffer size.
void framebuffer_size_callback(GLFWwindow * /*window*/, int width, int height) {
  glViewport(0, 0, width, height);
}

export namespace mka::graphic {

/// @brief High-level runtime state of a window instance.
enum class State { Inited, Running, Stopped, Terminated };

//-----
enum class KeyState { Pressed, Released };

enum class Key {
  Space,
  Apostrophe,
  Comma,
  Minus,
  Period,
  Slash,
  Num0,
  Num1,
  Num2,
  Num3,
  Num4,
  Num5,
  Num6,
  Num7,
  Num8,
  Num9,
  Semicolon,
  Equal,
  A,
  B,
  C,
  D,
  E,
  F,
  G,
  H,
  I,
  J,
  K,
  L,
  M,
  N,
  O,
  P,
  Q,
  R,
  S,
  T,
  U,
  V,
  W,
  X,
  Y,
  Z,
  LBracket,
  Backslash,
  RBracket,
  GraveAccent,
  Escape,
  Enter,
  Tab,
  Backspace,
  Insert,
  Delete,
  Right,
  Left,
  Down,
  Up,
  PageUp,
  PageDown,
  Home,
  End,
  CapsLock,
  ScrollLock,
  NumLock,
  PrintScreen,
  Pause,
  F1,
  F2,
  F3,
  F4,
  F5,
  F6,
  F7,
  F8,
  F9,
  F10,
  F11,
  F12,
  F13,
  F14,
  F15,
  F16,
  F17,
  F18,
  F19,
  F20,
  F21,
  F22,
  F23,
  F24,
  F25,
  Numpad0,
  Numpad1,
  Numpad2,
  Numpad3,
  Numpad4,
  Numpad5,
  Numpad6,
  Numpad7,
  Numpad8,
  Numpad9,
  NumpadDecimal,
  NumpadDivide,
  NumpadMultiply,
  NumpadSubtract,
  NumpadAdd,
  NumpadEnter,
  NumpadEqual,
  LeftShift,
  LeftCtrl,
  LeftAlt,
  LeftSuper,
  RightShift,
  RightCtrl,
  RightAlt,
  RightSuper,
  Menu,
  Unknown
};

class KeyboardEvent {
private:


friend class Window;
  static constexpr int KEY_COUNT = 121;

  struct CastElement {
    const size_t glfw;
    const Key key;
    const std::string keyName;
    KeyState state = KeyState::Released;
  };

  std::array<CastElement, KEY_COUNT> castTable {
      CastElement{GLFW_KEY_SPACE, Key::Space, "Space"},
      CastElement{GLFW_KEY_APOSTROPHE, Key::Apostrophe, "Apostrophe"},
      CastElement{GLFW_KEY_COMMA, Key::Comma, "Comma"},
      CastElement{GLFW_KEY_MINUS, Key::Minus, "Minus"},
      CastElement{GLFW_KEY_PERIOD, Key::Period, "Period"},
      CastElement{GLFW_KEY_SLASH, Key::Slash, "Slash"},
      CastElement{GLFW_KEY_0, Key::Num0, "Num0"},
      CastElement{GLFW_KEY_1, Key::Num1, "Num1"},
      CastElement{GLFW_KEY_2, Key::Num2, "Num2"},
      CastElement{GLFW_KEY_3, Key::Num3, "Num3"},
      CastElement{GLFW_KEY_4, Key::Num4, "Num4"},
      CastElement{GLFW_KEY_5, Key::Num5, "Num5"},
      CastElement{GLFW_KEY_6, Key::Num6, "Num6"},
      CastElement{GLFW_KEY_7, Key::Num7, "Num7"},
      CastElement{GLFW_KEY_8, Key::Num8, "Num8"},
      CastElement{GLFW_KEY_9, Key::Num9, "Num9"},
      CastElement{GLFW_KEY_SEMICOLON, Key::Semicolon, "Semicolon"},
      CastElement{GLFW_KEY_EQUAL, Key::Equal, "Equal"},
      CastElement{GLFW_KEY_A, Key::A, "A"},
      CastElement{GLFW_KEY_B, Key::B, "B"},
      CastElement{GLFW_KEY_C, Key::C, "C"},
      CastElement{GLFW_KEY_D, Key::D, "D"},
      CastElement{GLFW_KEY_E, Key::E, "E"},
      CastElement{GLFW_KEY_F, Key::F, "F"},
      CastElement{GLFW_KEY_G, Key::G, "G"},
      CastElement{GLFW_KEY_H, Key::H, "H"},
      CastElement{GLFW_KEY_I, Key::I, "I"},
      CastElement{GLFW_KEY_J, Key::J, "J"},
      CastElement{GLFW_KEY_K, Key::K, "K"},
      CastElement{GLFW_KEY_L, Key::L, "L"},
      CastElement{GLFW_KEY_M, Key::M, "M"},
      CastElement{GLFW_KEY_N, Key::N, "N"},
      CastElement{GLFW_KEY_O, Key::O, "O"},
      CastElement{GLFW_KEY_P, Key::P, "P"},
      CastElement{GLFW_KEY_Q, Key::Q, "Q"},
      CastElement{GLFW_KEY_R, Key::R, "R"},
      CastElement{GLFW_KEY_S, Key::S, "S"},
      CastElement{GLFW_KEY_T, Key::T, "T"},
      CastElement{GLFW_KEY_U, Key::U, "U"},
      CastElement{GLFW_KEY_V, Key::V, "V"},
      CastElement{GLFW_KEY_W, Key::W, "W"},
      CastElement{GLFW_KEY_X, Key::X, "X"},
      CastElement{GLFW_KEY_Y, Key::Y, "Y"},
      CastElement{GLFW_KEY_Z, Key::Z, "Z"},
      CastElement{GLFW_KEY_LEFT_BRACKET, Key::LBracket, "LBracket"},
      CastElement{GLFW_KEY_BACKSLASH, Key::Backslash, "Backslash"},
      CastElement{GLFW_KEY_RIGHT_BRACKET, Key::RBracket, "RBracket"},
      CastElement{GLFW_KEY_GRAVE_ACCENT, Key::GraveAccent, "GraveAccent"},
      CastElement{GLFW_KEY_ESCAPE, Key::Escape, "Escape"},
      CastElement{GLFW_KEY_ENTER, Key::Enter, "Enter"},
      CastElement{GLFW_KEY_TAB, Key::Tab, "Tab"},
      CastElement{GLFW_KEY_BACKSPACE, Key::Backspace, "Backspace"},
      CastElement{GLFW_KEY_INSERT, Key::Insert, "Insert"},
      CastElement{GLFW_KEY_DELETE, Key::Delete, "Delete"},
      CastElement{GLFW_KEY_RIGHT, Key::Right, "Right"},
      CastElement{GLFW_KEY_LEFT, Key::Left, "Left"},
      CastElement{GLFW_KEY_DOWN, Key::Down, "Down"},
      CastElement{GLFW_KEY_UP, Key::Up, "Up"},
      CastElement{GLFW_KEY_PAGE_UP, Key::PageUp, "PageUp"},
      CastElement{GLFW_KEY_PAGE_DOWN, Key::PageDown, "PageDown"},
      CastElement{GLFW_KEY_HOME, Key::Home, "Home"},
      CastElement{GLFW_KEY_END, Key::End, "End"},
      CastElement{GLFW_KEY_CAPS_LOCK, Key::CapsLock, "CapsLock"},
      CastElement{GLFW_KEY_SCROLL_LOCK, Key::ScrollLock, "ScrollLock"},
      CastElement{GLFW_KEY_NUM_LOCK, Key::NumLock, "NumLock"},
      CastElement{GLFW_KEY_PRINT_SCREEN, Key::PrintScreen, "PrintScreen"},
      CastElement{GLFW_KEY_PAUSE, Key::Pause, "Pause"},
      CastElement{GLFW_KEY_F1, Key::F1, "F1"},
      CastElement{GLFW_KEY_F2, Key::F2, "F2"},
      CastElement{GLFW_KEY_F3, Key::F3, "F3"},
      CastElement{GLFW_KEY_F4, Key::F4, "F4"},
      CastElement{GLFW_KEY_F5, Key::F5, "F5"},
      CastElement{GLFW_KEY_F6, Key::F6, "F6"},
      CastElement{GLFW_KEY_F7, Key::F7, "F7"},
      CastElement{GLFW_KEY_F8, Key::F8, "F8"},
      CastElement{GLFW_KEY_F9, Key::F9, "F9"},
      CastElement{GLFW_KEY_F10, Key::F10, "F10"},
      CastElement{GLFW_KEY_F11, Key::F11, "F11"},
      CastElement{GLFW_KEY_F12, Key::F12, "F12"},
      CastElement{GLFW_KEY_F13, Key::F13, "F13"},
      CastElement{GLFW_KEY_F14, Key::F14, "F14"},
      CastElement{GLFW_KEY_F15, Key::F15, "F15"},
      CastElement{GLFW_KEY_F16, Key::F16, "F16"},
      CastElement{GLFW_KEY_F17, Key::F17, "F17"},
      CastElement{GLFW_KEY_F18, Key::F18, "F18"},
      CastElement{GLFW_KEY_F19, Key::F19, "F19"},
      CastElement{GLFW_KEY_F20, Key::F20, "F20"},
      CastElement{GLFW_KEY_F21, Key::F21, "F21"},
      CastElement{GLFW_KEY_F22, Key::F22, "F22"},
      CastElement{GLFW_KEY_F23, Key::F23, "F23"},
      CastElement{GLFW_KEY_F24, Key::F24, "F24"},
      CastElement{GLFW_KEY_F25, Key::F25, "F25"},
      CastElement{GLFW_KEY_KP_0, Key::Numpad0, "Numpad0"},
      CastElement{GLFW_KEY_KP_1, Key::Numpad1, "Numpad1"},
      CastElement{GLFW_KEY_KP_2, Key::Numpad2, "Numpad2"},
      CastElement{GLFW_KEY_KP_3, Key::Numpad3, "Numpad3"},
      CastElement{GLFW_KEY_KP_4, Key::Numpad4, "Numpad4"},
      CastElement{GLFW_KEY_KP_5, Key::Numpad5, "Numpad5"},
      CastElement{GLFW_KEY_KP_6, Key::Numpad6, "Numpad6"},
      CastElement{GLFW_KEY_KP_7, Key::Numpad7, "Numpad7"},
      CastElement{GLFW_KEY_KP_8, Key::Numpad8, "Numpad8"},
      CastElement{GLFW_KEY_KP_9, Key::Numpad9, "Numpad9"},
      CastElement{GLFW_KEY_KP_DECIMAL, Key::NumpadDecimal, "NumpadDecimal"},
      CastElement{GLFW_KEY_KP_DIVIDE, Key::NumpadDivide, "NumpadDivide"},
      CastElement{GLFW_KEY_KP_MULTIPLY, Key::NumpadMultiply, "NumpadMultiply"},
      CastElement{GLFW_KEY_KP_SUBTRACT, Key::NumpadSubtract, "NumpadSubtract"},
      CastElement{GLFW_KEY_KP_ADD, Key::NumpadAdd, "NumpadAdd"},
      CastElement{GLFW_KEY_KP_ENTER, Key::NumpadEnter, "NumpadEnter"},
      CastElement{GLFW_KEY_LEFT_SHIFT, Key::LeftShift, "LeftShift"},
      CastElement{GLFW_KEY_LEFT_CONTROL, Key::LeftCtrl, "LeftCtrl"},
      CastElement{GLFW_KEY_LEFT_ALT, Key::LeftAlt, "LeftAlt"},
      CastElement{GLFW_KEY_LEFT_SUPER, Key::LeftSuper, "LeftSuper"},
      CastElement{GLFW_KEY_RIGHT_SHIFT, Key::RightShift, "RightShift"},
      CastElement{GLFW_KEY_RIGHT_CONTROL, Key::RightCtrl, "RightCtrl"},
      CastElement{GLFW_KEY_RIGHT_ALT, Key::RightAlt, "RightAlt"},
      CastElement{GLFW_KEY_RIGHT_SUPER, Key::RightSuper, "RightSuper"},
      CastElement{GLFW_KEY_MENU, Key::Menu, "Menu"}
  };

  CastElement *get(size_t glfw) {
    for (auto &e : castTable) {
      if (e.glfw == glfw)
        return &e;
    }
    return nullptr;
  }

  CastElement *get(Key key) {
    for (auto &e : castTable) {
      if (e.key == key)
        return &e;
    }
    return nullptr;
  }

public:
  bool isPressed(Key key) {
    auto e = get(key);
    if (!e)
      return false;

    return e->state == KeyState::Pressed;
  }

  bool isReleased(Key key) {
    auto e = get(key);
    if (!e)
      return false;

    return e->state == KeyState::Released;
  }

  void set(Key key, KeyState state) {
    if (auto e = get(key); e) {
      e->state = state;
    }
  }

  std::vector<Key> pressedKeys() const {
    std::vector<Key> keys;
    for (const auto &e : castTable) {
      if (e.state == KeyState::Pressed) {
        keys.emplace_back(e.key);
      }
    }
    return keys;
  }
};

//----
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
      : size(width, height), name(name), ctx(std::move(ctx)), window(nullptr) {
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
  }

  virtual void render(const glm::vec2 &size, const MouseEvent &mouse,
                      const KeyboardEvent &keyboard) = 0;

  /// @brief Projection updated whenever the window size changes.
  const glm::mat4 &getOrthographicProjection() const {
    return orthographicProjection;
  }

  /// @brief Run the blocking frame loop until close.
  int run() {
    if (state != State::Inited)
      return -1;

    state = State::Running;

    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
		
	  handleKeyboard();

      ctx->makeCurrent();

      render(size, mouseEvent, keyboardEvent);

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

  const State &getState() { return state; }

private:
  glm::vec2 size = {0.0, 0.0};
  std::string name;
  std::unique_ptr<Context> ctx;
  State state = State::Terminated;
  GLFWwindow *window = nullptr;
  glm::mat4 orthographicProjection;
  MouseEvent mouseEvent;
  KeyboardEvent keyboardEvent;

private:
  void handleKeyboard() {
    for (auto &i : keyboardEvent.castTable) {
      i.state = glfwGetKey(window, i.glfw) == GLFW_RELEASE ? KeyState::Released
                                                           : KeyState::Pressed;
    }
  }
};
} // namespace mka::graphic
