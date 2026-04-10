module;

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <algorithm>
export module mka.graphic.view;
import mka.graphic.opengl.renderer;

export namespace mka::graphic {

	class View {
		public:

			// Constructeur / Destructeur
			View() = default;
			virtual ~View() = default;
			
			const View* getParent() const { return parent; }

			// --- Gestion des enfants ---
			void addChild(std::unique_ptr<View> child) {
				child->parent = this;
				children.push_back(std::move(child));
			}

			void removeChild(View* child) {
				children.erase(
					std::remove_if(children.begin(), children.end(),
						[&](const std::unique_ptr<View>& c){ return c.get() == child; }),
					children.end()
				);
			}

			const std::vector<std::unique_ptr<View>>& getChildren() const {
				return children;
			}

			virtual void draw(class Renderer &renderer) = 0;

			glm::vec2 getPosition() const { return glm::vec2{geometry.x, geometry.y}; }
			glm::vec2 getSize() const { return glm::vec2{geometry.z, geometry.w}; }

			void setPosition(const glm::vec2& p) { 
				geometry.x = p.x; 
				geometry.y = p.y;
			}

			void setSize(const glm::vec2& s) { 
				geometry.z = s.x;
				geometry.w = s.y; 
			}

			void setVisible(bool v) { visible = v; }
			bool isVisible() { return visible; }
			
			int zIndex = 0;

		protected:
			glm::vec4 geometry = {0.0f, 0.0f, 0.0f, 0.0f};
			std::vector<std::unique_ptr<View>> children;
				
		private:
			View* parent = nullptr;
			bool visible = true;
		};

} // mka::graphic
