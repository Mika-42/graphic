module;
#include <string>
#include <vector>
export module mka.graphic.keyboardview;
export import mka.graphic.keyboard; //TODO find a way to import only enum

export namespace mka::graphic {

struct KeyboardEventView {
public:
  KeyboardEventView(KeyboardEvent &kb) : k(kb) {}
  bool isPressed(Key key) const { return k.isPressed(key); }

  bool isReleased(Key key) const { return k.isReleased(key); }

  std::vector<Key> pressedKeys() const { return k.pressedKeys(); }

  const std::string getName(Key key) const { return k.getName(key); }

private:
  KeyboardEvent &k;
};

} // namespace mka::graphic
