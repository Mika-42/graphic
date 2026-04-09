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
		Left, Right, Center, Top, Bottom
	};

	enum class Orientation {
		Vertical, Horizontal
	};

	class StackView : public View {
		public:
		
		void setSize(const glm::vec2& s) = delete;

		void draw(Renderer& renderer) override {
			switch(orientation) {
				case Orientation::Vertical: vlayout(); break;
				case Orientation::Horizontal: hlayout(); break;
				default: vlayout();
			}

			View::draw(renderer);
		}
		
		void setGap(float v) {
			gap = sanitizeFloat(v, 0.0f);
		}

		void setAlign(Align a) {
			align = a;
		}

		void setOrientation(Orientation o) {
			orientation = o;
		}

		const float& getGap() { return gap; }
		const Align& getAlign() { return align; }
		const Orientation& getOrientation() { return orientation; }

		private:
			void vlayout() {

				float offset = 0.0f;
				for(auto& child : children) {
					geometry.z = glm::max(geometry.z, child->getSize().x);
				
					float leftOffset = 0.0f;

					switch(align) {
						case Align::Center: leftOffset = 0.5f * (geometry.z - child->getSize().x); break;
						case Align::Left:	leftOffset = 0.0f; break;
						case Align::Right:	leftOffset = geometry.z - child->getSize().x; break;
						default:			leftOffset = 0.0f;
					}

					child->setPosition({leftOffset + geometry.x, geometry.y + offset});
					offset += child->getSize().y + gap;
				}
				geometry.w = offset;
			}

			void hlayout() {

				float offset = 0.0f;
				for(auto& child : children) {
					geometry.w = glm::max(geometry.w, child->getSize().y);
				
					float bottomOffset = 0.0f;

					switch(align) {
						case Align::Center: bottomOffset = 0.5f * (geometry.w - child->getSize().y); break;
						case Align::Bottom:	bottomOffset = geometry.w - child->getSize().y; break;
						case Align::Top: bottomOffset = 0.0f; break;
						default:			bottomOffset = geometry.w - child->getSize().y;
					}

					child->setPosition({geometry.x + offset, geometry.y + bottomOffset});
					offset += child->getSize().x + gap;
				}
				geometry.z = offset;
			}

		private:
			Align align = Align::Left;
			Orientation orientation = Orientation::Vertical;
			float gap = 0.0f;
	};

} // mka::graphic
