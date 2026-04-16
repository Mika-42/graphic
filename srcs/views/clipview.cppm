module;
#include <glm/glm.hpp>
#include "debug.hpp"

export module mka.graphic.view.clipview;
import mka.graphic.view;
import mka.graphic.opengl.rectangle;
import mka.graphic.opengl.renderer;

export namespace mka::graphic {

class ClipView : public View {
public:
  ClipView() : View() {}

  // 🔥 Une seule méthode : met à jour geometry ET clip en même temps
  void setClip(const glm::vec4& geometry, const glm::vec4& radius) {
//	setRelativePosition({geometry.x, geometry.y});
//	setAbsolutePosition({geometry.x, geometry.y});
//	setSize({geometry.z, geometry.w});  // Met à jour View::geometry
    this->geometry = geometry;
	setClipGeometry(geometry);  // Active clip avec même géométrie
    setClipRadius(radius);
    markDirty();
  }

  void draw(Renderer &renderer) override {
    // Debug visuel si clip actif
    if (shouldClip()) {
      Rectangle clipDebug{
        .geometry = geometry,
        .radius = getClipRadius(),
        .backgroundColorA = {1.0f, 0.0f, 0.0f, 0.1f},
        .borderColor = {1.0f, 0.0f, 0.0f, 0.5f},
        .borderThickness = 2.0f
      };
      renderer.add(clipDebug);
    }
    
    View::draw(renderer);  // ClipScope gère tout !
  }

private:
  void layout() override {
    // Positionne les enfants (comme FloatView)
    for (auto &child : children) {
      if (child) {
        const glm::vec2 relPos = child->getRelativePosition();
        const glm::vec2 absPos = getAbsolutePosition() + relPos;
        child->setAbsolutePosition(absPos);
      }
    }

  }
 
};
} // namespace mka::graphic
