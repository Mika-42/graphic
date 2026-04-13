module;

export module mka.grphic.view.clipview;
import mka.graphic.view;
import mka.graphic.opengl.rectangle;

export namespace mka::graphic {

	class ClipView : public View {
		public:
		private:
			Rectangle viewport;
	};
} // namespace mka::graphic
