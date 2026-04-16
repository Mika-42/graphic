#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <iostream>
#include <memory>
#include <string>
#include "debug.hpp"

import mka.graphic.window;
import mka.graphic.opengl.renderer;
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
			View::draw(renderer);

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
  Window app(1200, 800, "ClipView Nesting Test", std::move(ctx));

  // 🟦 ROOT FloatView - Conteneur flottant principal
  auto root = std::make_shared<FloatView>();
  
  // 🔴 CLIP 1 - ClipView principal (grand cercle rouge)
  auto clip1 = std::make_shared<ClipView>();
  //clip1->setRelativePosition({50.0f, 50.0f});
  clip1->setClip({0.0f, 0.0f, 400.0f, 400.0f},glm::vec4{200.0f}); // Grand cercle rouge

  // 🟨 CLIP 2 - ClipView imbriqué (carré jaune avec coins arrondis)
  auto clip2 = std::make_shared<ClipView>();
  //clip2->setRelativePosition({100.0f, 100.0f});
  clip2->setClip({0.0f, 0.0f, 250.0f, 250.0f}, {50.0f, 50.0f, 80.0f, 80.0f}); // Coins différents

  // 🟢 CLIP 3 - ClipView le plus imbriqué (petit cercle vert)
  auto clip3 = std::make_shared<ClipView>();
  //clip3->setRelativePosition({80.0f, 80.0f});
  clip3->setClip({0.0f, 0.0f, 120.0f, 120.0f}, glm::vec4{60.0f}); // Petit cercle parfait

  // Contenu TEST 1 - Rectangle A (doit être COMPLETEMENT coupé par clip3)
  auto rectA = std::make_shared<A>(glm::vec4{1.0f, 0.0f, 0.0f, 1.0f}); // Rouge
  rectA->setRelativePosition({-20.0f, -20.0f});
  rectA->setSize({80.0f, 80.0f});

  // Contenu TEST 2 - Rectangle B (partiellement visible dans clip3)
  auto rectB = std::make_shared<A>(glm::vec4{0.0f, 1.0f, 0.0f, 1.0f}); // Vert
  rectB->setRelativePosition({20.0f, 20.0f});
  rectB->setSize({100.0f, 100.0f});

  // Contenu TEST 3 - Rectangle C (visible dans clip2 mais pas clip3)
  auto rectC = std::make_shared<A>(glm::vec4{0.0f, 0.0f, 1.0f, 1.0f}); // Bleu
  rectC->setRelativePosition({100.0f, 50.0f});
  rectC->setSize({120.0f, 80.0f});

  // Contenu TEST 4 - Rectangle D (visible dans clip1 mais pas clip2)
  auto rectD = std::make_shared<A>(glm::vec4{1.0f, 1.0f, 0.0f, 1.0f}); // Jaune
  rectD->setRelativePosition({-50.0f, 150.0f});
  rectD->setSize({150.0f, 100.0f});

  // Contenu TEST 5 - Rectangle E (visible partout - référence)
  auto rectE = std::make_shared<A>(glm::vec4{1.0f, 0.0f, 1.0f, 1.0f}); // Magenta
  rectE->setRelativePosition({400.0f, 100.0f});
  rectE->setSize({80.0f, 80.0f});

  // 🔧 ASSEMBLAGE IMBRIQUÉ (ORDRE IMPORTANT)
  // clip3 contient A et B
  clip3->addChild(rectA);
  clip3->addChild(rectB);
  
 // clip2 contient clip3 et C  
  clip2->addChild(clip3);
  clip2->addChild(rectC);
  
//  clip1 contient clip2 et D
  clip1->addChild(clip2);
  clip1->addChild(rectD);
  
  // root contient clip1 et E
  root->addChild(clip1);
  root->addChild(rectE);

  /*
  HIERARCHIE FINALE:
  
  FloatView (root)
  ├── ClipView1 🔴 (grand cercle 400x400)
  │   ├── ClipView2 🟨 (carré 250x250)
  │   │   ├── ClipView3 🟢 (petit cercle 120x120)
  │   │   │   ├── Rect A 🔴 (COMPLÈTEMENT coupé)
  │   │   │   └── Rect B 🟢 (partiellement visible)
  │   │   └── Rect C 🔵 (visible dans clip2)
  │   └── Rect D 🟡 (visible dans clip1)
  └── Rect E 🟣 (visible partout)
  */

  app.setBackgroundColor({0.1f, 0.1f, 0.15f, 1.0f});
  app.setRoot(root);

  return app.run();
}
