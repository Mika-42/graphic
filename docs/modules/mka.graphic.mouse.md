# Module `mka.graphic.mouse`

[⬅ Back to index](./README.md)

This module stores mouse state shared by the window callbacks and render loop.

## Enums
- `MouseButton`: `Left`, `Middle`, `Right`.
- `MouseState`: `Pressed`, `Released`, `ScrollUp`, `ScrollDown`.

## Class `MouseEvent`
Holds current button states and pointer values.

### Methods
- `bool isPressed(MouseButton btn) const`
- `bool isReleased(MouseButton btn) const`
- `bool isScrollUp(MouseButton btn) const`
- `bool isScrollDown(MouseButton btn) const`
- `void set(MouseButton button, MouseState state)`

### Public fields
- `glm::vec2 scroll`: latest wheel delta.
- `glm::vec2 position`: latest cursor position in window coordinates.

### Runtime flow
`Window` updates `MouseEvent` from GLFW callbacks, then passes a read-only `MouseEventView` into `render(...)` each frame.
