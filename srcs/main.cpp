#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <iostream>
#include <memory>

import mka.graphic.window;
import mka.graphic.opengl.renderer;
import mka.graphic.renderlist;
import mka.graphic.view;
import mka.graphic.view.stackview;
import mka.graphic.view.floatview;
import mka.graphic.sanitize;

using namespace mka::graphic;

enum class KeyColor { Black, White };
enum class KeyNote { C, D, E, F, G, A, B };
enum class KeySymbol { Sharp, Bemol, None };
/*
class PianoOctave : public View {
	public:
  glm::vec4 whiteKeyColor;
  glm::vec4 blackKeyColor;
  int octave = 0;

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

  void draw(Renderer &renderer) override {

    const float keyHeight = geometry.w / 10.5f;
    const float radius = keyHeight * 0.25f;

    for (auto &k : keyPattern) {
      const bool isBlackKey = (k.color == KeyColor::Black);
      const glm::vec4 keyColor = isBlackKey ? blackKeyColor : whiteKeyColor;
      const float keyWidth = geometry.z * (isBlackKey ? 0.6f : 1.0f);

      Rectangle key{
          .geometry = {geometry.x, 0.0f, keyWidth, k.height * keyHeight},
          .radius = {radius, radius, 0, 0},
          .backgroundColorA = keyColor,
          .backgroundColorB = keyColor,
      };

      key.geometry.y =
          geometry.y + geometry.w - (k.position + k.height) * keyHeight;

      renderer.add(key);
    }
  }
};
*/

class A : public View {
	public:
		A(glm::vec4 c) : cc(c) {
				aa.backgroundColorA = cc;
				aa.backgroundColorB = cc;	
		}
	
		void draw(Renderer &renderer) override {
			
			aa.geometry = this->geometry;

			if(isMouseFocused()) {
				aa.backgroundColorA = glm::vec4(0.0f, 1.0, 0.0, 1.0);
				aa.backgroundColorB = glm::vec4(0.0f, 1.0, 0.0, 1.0);
			} else {
				aa.backgroundColorA = cc;
				aa.backgroundColorB = cc;	
			}

			renderer.add(aa);
		}

		bool contain(const glm::vec2& mouse) const override {
			return (distance(aa, mouse) <= 0.0f);
		}

		void onMouseEvent(const MouseEventView &/*mouse*/) override {
		}

	private:
		glm::vec4 cc;
		Rectangle aa;
};

int main() {
  auto ctx = mka::graphic::createContext(mka::graphic::API::OpenGL,
                                         mka::graphic::Loader::Glad);
  Window app(800, 600, "Example", std::move(ctx));


//  static constexpr size_t OCTAVE_COUNT = 3;
  FloatView s;
  auto a1 = std::make_unique<A>(glm::vec4{1.0, 0.0, 0.0, 0.5});
  auto a2 = std::make_unique<A>(glm::vec4{1.0, 1.0, 0.0, 0.5});

  s.setPosition({400, 100});
  
  a1->setPosition({0, 0}).setSize({100, 200});
  
  a2->setPosition({50, 50}).setSize({100, 200});
  
  a1->zIndex = 1;
  a2->zIndex = 1000;

  s.addChild(std::move(a1)).addChild(std::move(a2));

  /*StackView s;
  s.setPosition({200, 200});
	s.setOrientation(Orientation::Horizontal);
	s.setAlign(Align::Center);
	s.setGap(5);
	s.reverse(false);

    for (size_t o = 0; o < OCTAVE_COUNT; ++o) {
      auto octave = std::make_unique<PianoOctave>();
		
	  octave->octave = o;
      octave->setPosition({0, o * 200});
	  octave->setSize({50 + 50 * o, 200 + 10 * o});
      octave->whiteKeyColor = {1.0, 1.0, 1.0, 1.0};
      octave->blackKeyColor = {0.0, 0.0, 0.0, 1.0};
	  s.addChild(std::move(octave));
    }*/

	app.setBackgroundColor({0.25, 0.25, 0.25, 1.0});
	app.setRoot(s);

  return app.run();
}
