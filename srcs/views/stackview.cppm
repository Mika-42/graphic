module;

#include <vector>
#include <memory>
#include <glm/glm.hpp>

export module mka.graphic.view.stackview;
import mka.graphic.view;
import mka.graphic.sanitize;
import mka.graphic.opengl.renderer;

export namespace mka::graphic {
	
	enum class Align {
		Left, Right, Center
	};

	class StackView : public View {
		public:
		
		void setSize(const glm::vec2& s) = delete;

		void draw(Renderer& renderer) override {
			float offset = 0.0f;
			for(auto& child : children) {
				geometry.z = glm::max(geometry.z, child->getSize().x);
			
				float leftOffset = 0.0f;

				switch(align) {
					case Align::Center: leftOffset = 0.5f * (geometry.z - child->getSize().x); break;
					case Align::Left:	leftOffset = 0.0f; break;
					case Align::Right:	leftOffset = geometry.z - child->getSize().x; break;
					default: leftOffset = 0.0f;
				}

				child->setPosition({leftOffset + geometry.x, geometry.y + offset});
				offset += child->getSize().y + gap;
			}
			geometry.w = offset;
			View::draw(renderer);
		}
		
		void setGap(float v) {
			gap = sanitizeFloat(v, 0.0f);
		}

		void setAlign(Align a) {
			align = a;
		}

		private:
			Align align = Align::Left;
			float gap = 0.0f;
	};

} // mka::graphic
