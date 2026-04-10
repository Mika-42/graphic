module;

export module mka.graphic.view.gridview;
import mka.graphic.view;
import mka.graphic.opengl.renderer;

export namespace mka::graphic {

	/*
	 *     (0, 0)  +------+----------------+
	 *             |  A   |  B             |
	 *             +------+----------------+
	 *             |      |                |
	 *             |  C   |   D            |
	 *             |      |                |
	 *             +------+----------------+
	 *
	 * col : x, 50, 100
	 * row : y, 30, 10
	 *
	 *	   (row, col, rowspan, colspan)
	 * A = (0, 0, 1, 1)
	 * B = (0, 1, 1, 1)
	 * C = (1, 0, 1, 1)
	 * D = (1, 1, 1, 1)
	 * 
	 * GridView view;
	 * view.columns({50, 100});
	 * view.rows({30, 10});
	 * view.add(std::move(element), {0, 0});
	 */

	class GridView : public View {
		
	};

} // namespace mka::graphic
