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
private:
  using View::addChild;
  using View::getSize;
  using View::removeChild;
  using View::setSize;

public:
  GridView &setColumns(const std::vector<Size> &c) {
    columns = c;
    return *this;
  }

  GridView &setRows(const std::vector<Size> &r) {
    rows = r;
    return *this;
  }

  GridView &addChild(std::unique_ptr<View> child, size_t row, size_t col,
                     size_t rowSpan = 1, size_t colSpan = 1) {
    childCell[child.get()] = {
        .row = row, .column = column, .rowSpan = rowSpan, .colSpan = colSpan};
    return View::addChild(std::move(child));
  }

  GridView &move(View *child, size_t row, size_t col, size_t rowSpan = 1,
                 size_t colSpan = 1) {
    if (childCell.contains(child)) {

      childCell[child.get()] = {
          .row = row, .column = column, .rowSpan = rowSpan, .colSpan = colSpan};
    }

    layout();
    return *this;
  }

  virtual View &removeChild(View *child) override {
    childCell.erase(child);
    return View::removeChild(child);
  }

  GridView &setRowGap(float v) {
    gaps.x = sanitizeFloat(v, 0.0f);
    return *this;
  }

  GridView &setColumnGap(float v) {
    gaps.y = sanitizeFloat(v, 0.0f);
    return *this;
  }

  virtual glm::vec2 getSize() override {
    layout();
    return View::getSize();
  }

  const glm::vec2 &getGaps() const { return gaps; }

private:
  void layout() {

	if (columns.empty() || rows.empty()) {
		geometry.z = geometry.w = 0;
		return;
	}

    for (auto &child : children) {
      // TODO
    }
  }

  std::vector<float> getColumnWidths() {
	std::vector<float> widths(columns.size());

	return widths;
  }

private:
  std::vector<Size> rows;
  std::vector<Size> columns;
  glm::vec2 gaps = {0, 0};

  struct GridCell {
    size_t row;
    size_t column;
    size_t rowSpan;
    size_t colSpan;
  };
  std::unordered_map<View *, GridCell> childCell;
};

} // namespace mka::graphic
