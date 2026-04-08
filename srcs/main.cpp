#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <iostream>
#include <memory>

import mka.graphic.window;
import mka.graphic.opengl.renderer;

using namespace mka::graphic;

enum class KeyColor { Black, White };
enum class KeyNote { C, D, E, F, G, A, B };
enum class KeySymbol { Sharp, Bemol, None };

struct PianoOctave {
  glm::vec4 geometry;
  glm::vec4 whiteKeyColor;
  glm::vec4 blackKeyColor;
  int octave = 0;
  gl::Rectangle keys[12];

  struct KeyInfo {
    KeyNote note;
    KeySymbol symbol;
    KeyColor color;
    const float height;
    const float position;
  };

  const KeyInfo keyPattern[12] = {
      KeyInfo{KeyNote::C, KeySymbol::None, KeyColor::White, 1.5f, 0.0f},  // C
      KeyInfo{KeyNote::D, KeySymbol::None, KeyColor::White, 1.5f, 1.5f},  // D
      KeyInfo{KeyNote::E, KeySymbol::None, KeyColor::White, 1.5f, 3.0f},  // E
      KeyInfo{KeyNote::F, KeySymbol::None, KeyColor::White, 1.5f, 4.5f},  // F
      KeyInfo{KeyNote::G, KeySymbol::None, KeyColor::White, 1.5f, 6.0f},  // G
      KeyInfo{KeyNote::A, KeySymbol::None, KeyColor::White, 1.5f, 7.5f},  // A
      KeyInfo{KeyNote::B, KeySymbol::None, KeyColor::White, 1.5f, 9.0f},  // B
      KeyInfo{KeyNote::C, KeySymbol::Sharp, KeyColor::Black, 1.0f, 1.0f}, // C#
      KeyInfo{KeyNote::D, KeySymbol::Sharp, KeyColor::Black, 1.0f, 2.5f}, // D#
      KeyInfo{KeyNote::F, KeySymbol::Sharp, KeyColor::Black, 1.0f, 5.5f}, // F#
      KeyInfo{KeyNote::G, KeySymbol::Sharp, KeyColor::Black, 1.0f, 7.0f}, // G#
      KeyInfo{KeyNote::A, KeySymbol::Sharp, KeyColor::Black, 1.0f, 8.5f}  // A#
  };

  void evalPos() {

    const float keyHeight = geometry.w / 10.5f;

    for (size_t i = 0; i < 12; ++i) {
      const bool isBlackKey = (keyPattern[i].color == KeyColor::Black);
      keys[i].geometry.z = geometry.z * (isBlackKey ? 0.6f : 1.0f);
      keys[i].geometry.w = keyPattern[i].height * keyHeight;
      keys[i].geometry.x = geometry.x;
      keys[i].geometry.y =
          geometry.y +
          (geometry.w -
           (keyPattern[i].position + keyPattern[i].height) * keyHeight);

      keys[i].backgroundColorA = keys[i].backgroundColorB =
          isBlackKey ? blackKeyColor : whiteKeyColor;

      const float radius = keyHeight * 0.25f;
      keys[i].radius = {radius, radius, 0, 0};
    }
  }
};

class MinimalApp : public mka::graphic::Window {
public:
  MinimalApp(std::unique_ptr<mka::graphic::Context> ctx)
      : Window(800, 600, "Minimal", std::move(ctx)) {

    for (size_t o = 0; o < OCTAVE_COUNT; ++o) {
      octave[o].octave = o;
      octave[o].geometry = {0, o * 200, 50, 200};
      octave[o].whiteKeyColor = {1.0, 1.0, 1.0, 1.0};
      octave[o].blackKeyColor = {0.0, 0.0, 0.0, 1.0};
    }
  }

  void render(const glm::vec2 & /*size*/, const MouseEventView & /*mouse*/,
              const KeyboardEventView & /*keyboard*/,
              const Time & /*time*/) override {

    const glm::mat4 projection = getOrthographicProjection();
    renderer.setBackgroundColor({0.15, 0.15, 0.15, 1});

    for (size_t o = 0; o < OCTAVE_COUNT; ++o) {
      octave[o].evalPos();

      if (octave[o].octave == 11) {

        for (size_t i = 0; i < 12; ++i) {
          const bool isGSharp =
              (octave[o].keyPattern[i].note == KeyNote::G &&
               octave[o].keyPattern[i].symbol == KeySymbol::Sharp);
          const bool isA = octave[o].keyPattern[i].note == KeyNote::A;
          const bool isB = octave[o].keyPattern[i].note == KeyNote::B;

          if (!isGSharp && !isA && !isB) {
            renderer.add(std::move(octave[o].keys[i]));
          }
        }
      }

      for (size_t i = 0; i < 12; ++i) {
        renderer.add(std::move(octave[o].keys[i]));
      }
    }
    renderer.draw(projection);
  }

private:
  static constexpr size_t OCTAVE_COUNT = 11;
  PianoOctave octave[OCTAVE_COUNT];
  mka::graphic::gl::Renderer<1024> renderer;
};

int main() {
  auto ctx = mka::graphic::createContext(mka::graphic::API::OpenGL,
                                         mka::graphic::Loader::Glad);
  MinimalApp app(std::move(ctx));
  return app.run();
}
