module;

export module mka.graphic.view.gridview;
import mka.graphic.view;
import mka.graphic.opengl.renderer;
import mka.graphic.sanitize;

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

enum class Unit { Px, Percent, Fr };

struct Size {
  Unit unit;
  float value;
};

const Size px(float v) { return {Unit::Px, sanitizeFloat(v, 0.0f)}; }

const Size pct(float v) { return {Unit::Percent, sanitizeFloat(v, 0.0f)}; }

const Size fr(float v) { return {Unit::Fr, sanitizeFloat(v, 0.0f)}; }

class GridView : public View {

  View& setSize(const glm::vec2 &s) = delete;

  GridView& setColumns(const std::vector<Size> &c) { columns = c; return *this; }

  GridView& setRows(const std::vector<Size> &r) { rows = r; return *this;}

  GridView& addChild(std::unique_ptr<View> child, size_t row, size_t col,
                size_t rowSpan = 1, size_t colSpan = 1) {return *this;}

  GridView& setRowGap(float v) { gaps.x = sanitizeFloat(v, 0.0f); return *this;}

  GridView setColumnGap(float v) { gaps.y = sanitizeFloat(v, 0.0f); return *this;}

  const glm::vec2& getGaps() const { return gaps; }

private:
  std::vector<Size> rows;
  std::vector<Size> columns;
  glm::vec2 gaps = {0, 0};
};

} // namespace mka::graphic
