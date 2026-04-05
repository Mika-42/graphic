module;
#include <GLFW/glfw3.h>
#include <string>
#include <array>
#include <vector>

export module mka.graphic.keyboard;

export namespace mka::graphic {

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
public:
  static constexpr int KEY_COUNT = 121;

  struct CastElement {
    const size_t glfw;
    const Key key;
    const std::string keyName;
    KeyState state = KeyState::Released;
  };
  std::array<CastElement, KEY_COUNT> castTable{
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
      CastElement{GLFW_KEY_MENU, Key::Menu, "Menu"}};

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

  const CastElement *get(size_t glfw) const {
    for (auto &e : castTable) {
      if (e.glfw == glfw)
        return &e;
    }
    return nullptr;
  }

  const CastElement *get(Key key) const {
    for (auto &e : castTable) {
      if (e.key == key)
        return &e;
    }
    return nullptr;
  }
  
  const std::string getName(Key key) const {

    for (auto &e : castTable) {
      if (e.key == key)
        return e.keyName;
    }
    return {};
  }
public:
  bool isPressed(Key key) const {
    auto e = get(key);
    if (!e)
      return false;

    return e->state == KeyState::Pressed;
  }

  bool isReleased(Key key) const {
    auto e = get(key);
    if (!e)
      return false;

    return e->state == KeyState::Released;
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
} // namespace mka::graphic
