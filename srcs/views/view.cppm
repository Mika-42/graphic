module;

#include <glm/glm.hpp>

export module mka.graphic.view.base;
import mka.graphic.rectangle;

export namespace mka::graphic {

	class View {
		public:
			int zIndex;
			glm::vec4 geometry;
		protected:
			View* parent = nullptr;
			std::vector<Rectangle> childs;
	};

} // mka::graphic
