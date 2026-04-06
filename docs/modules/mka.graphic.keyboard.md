# Module `mka.graphic.keyboard`

[⬅ Back to index](./README.md)

This module provides a stable per-frame keyboard state table.

## Enums
- `KeyState`: `Pressed` or `Released`.
- `Key`: engine-level key identifiers mapped from GLFW key codes.

## Class `KeyboardEvent`
Contains a fixed-size key mapping table (`castTable`) where each entry stores:
- GLFW key code,
- engine key enum,
- display name,
- current state.

### Main methods
- `bool isPressed(Key key) const`
- `bool isReleased(Key key) const`
- `std::vector<Key> pressedKeys() const`
- `const std::string getName(Key key) const`

### Notes
- States are updated by `Window::handleKeyboard()` once every frame.
- Your render code should query `KeyboardEventView` from `Window::render(...)` rather than calling GLFW directly.
