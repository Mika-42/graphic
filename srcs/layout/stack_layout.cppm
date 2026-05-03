module;
#include <glm/glm.hpp>
export module mka.graphic.layout.stack;
import mka.graphic.view;

export namespace mka::graphic {
    class StackLayout : public View::Layout {
    public:
        enum class Direction { Vertical, Horizontal };
        enum class Align { Start, Center, End, Stretch };
		
        virtual void apply() override {

			float yOffset = 0.0f;
			float yFreeSpace = geometry().w;

			for(auto &child : children()) {
				child->setPosition(glm::vec2{0.0f, yOffset});
				resolveSize(*child, glm::vec2{0.0f, yFreeSpace}); //TODO get emty space
				
				const float childHeight = child->getGeometry().w;

				yOffset += childHeight;
				yFreeSpace -= childHeight;
			}
		}
		
    };
} // namespace mka::graphic

