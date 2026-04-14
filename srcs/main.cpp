#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <iostream>
#include <memory>
#include <string>
#include "debug.hpp"

import mka.graphic.window;
import mka.graphic.opengl.renderer;
import mka.graphic.renderlist;
import mka.graphic.view;
import mka.graphic.view.stackview;
import mka.graphic.view.floatview;
import mka.graphic.view.gridview;
import mka.graphic.view.clipview;
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
		A() = default;
		A(glm::vec4 c) : cc(c) {
				aa.backgroundColorA = cc;
				aa.backgroundColorB = cc;
				
		}
	
		void draw(Renderer &renderer) override {
			
			aa.geometry = this->geometry;
			aa.radius = glm::vec4(90.0f);

			if(isMouseFocused()) {
				aa.backgroundColorA = glm::vec4(0.0f, 1.0, 0.0, 1.0);
				aa.backgroundColorB = glm::vec4(0.5f, 1.0, 1.0, 1.0);
			} else {
				aa.backgroundColorA = cc;
				aa.backgroundColorB = cc;	
			}

			aa.borderColor = glm::vec4(1.0f);
			aa.shadowColor = {0.5f, 0.25f, 1.0f, 0.5f};
			aa.shadowOffset = {10.0f, 10.0f};
			aa.gradientAngle = 60.0f;
			aa.shadowSoftness = 50.0f;
			aa.shadowSpread = 10.0f;
			aa.borderThickness = 5.0f;

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

auto r = std::make_shared<FloatView>();
auto f = std::make_shared<ClipView>();

f->setRelativePosition({200.0f, 200.0f});
f->setSize({800.0f, 500.0f});
f->setRadius({100.0f, 50.0f, 30.0f, 50.0f});

auto s = std::make_shared<StackView>();
 
s->setAbsolutePosition({200.0f, 200.0f});
s->setOrientation(Orientation::Vertical);
s->setAlign(Align::Center);
s->setGap(5);

auto a = std::make_shared<A>(glm::vec4{0.45f, 0.55f, 0.75f, 1.0f});
a->setSize({200.0f, 100.0f}); 

auto b = std::make_shared<A>(glm::vec4{0.45f, 0.55f, 0.75f, 1.0f});
b->setSize({200.0f, 100.0f});

auto c = std::make_shared<A>(glm::vec4{0.45f, 0.55f, 0.75f, 1.0f});
c->setSize({200.0f, 100.0f}); 

auto d = std::make_shared<A>(glm::vec4{0.45f, 0.55f, 0.75f, 1.0f});
d->setSize({200.0f, 100.0f});

auto e = std::make_shared<A>(glm::vec4{0.45f, 0.55f, 0.75f, 1.0f});
e->setSize({200.0f, 100.0f}); 

s->addChild(a);
s->addChild(b);
s->addChild(c);
s->addChild(d);
s->addChild(e);

f->addChild(s);

r->addChild(f);



//	auto pp = std::make_shared<A>(glm::vec4{1.0f});
//	auto p = r->addChild(std::move(pp));


//	r->addChild(std::move(f));
//	p.setRelativePosition({250.0f, 250.0f});
//	p.setSize({200.0f, 200.0f});
	/*
    for (size_t o = 0; o < OCTAVE_COUNT; ++o) {
      auto octave = std::make_shared<PianoOctave>();
		
	  octave->octave = o;
      octave->setPosition({0, o * 200});
	  octave->setSize({50 + 50 * o, 200 + 10 * o});
      octave->whiteKeyColor = {1.0, 1.0, 1.0, 1.0};
      octave->blackKeyColor = {0.0, 0.0, 0.0, 1.0};
	  s.addChild(std::move(octave));
    }*/

	app.setBackgroundColor({0.25, 0.25, 0.25, 1.0});
	app.setRoot(r);

  return app.run();
}
