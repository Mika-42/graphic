module;

#include <glm/glm.hpp>

export module mka.graphic.view;

export namespace mka::graphic {

	class View {
		public:
			int zIndex;
			glm::vec4 geometry;
		private:
	};

} // mka::graphic
