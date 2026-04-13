module;
#include "debug.hpp"
#include <algorithm>
#include <cstddef>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

export module mka.graphic.view.gridview;
import mka.graphic.view;
import mka.graphic.opengl.renderer;
import mka.graphic.sanitize;

namespace mka::graphic {

// Pré-calcul des positions cumulées O(1) accès
struct TrackCache {
  std::vector<float> positions; // positions cumulées
  std::vector<float> sizes;     // tailles individuelles
  float totalSize = 0.0f;
  float overflow = 0.0f;
};
} // namespace mka::graphic

export namespace mka::graphic {

enum class Unit : uint8_t { Px, Fr };

struct GridCell {
  size_t row, col, rowSpan, colSpan;
};

struct Size {
  Unit unit;
  float value;
};

const Size px(float v) { return {Unit::Px, sanitizeFloat(v, 0.0f)}; }
const Size fr(float v) { return {Unit::Fr, sanitizeFloat(v, 0.0f)}; }

class GridView : public View {
private:
  using View::addChild;
  using View::removeChild;

  mutable TrackCache colCache, rowCache;
  std::unordered_map<View *, GridCell> childCell;

public:
  GridView &setColumns(const std::vector<Size> &c) {
    columns = c;
    markDirty();
    return *this;
  }
  GridView &setRows(const std::vector<Size> &r) {
    rows = r;
    markDirty();
    return *this;
  }

  GridView &extendColumns(const Size &s) {
    columns.emplace_back(s);
    markDirty();
    return *this;
  }

  GridView &extendRows(const Size &s) {
    rows.emplace_back(s);
    markDirty();
    return *this;
  }

  GridView &addChild(std::unique_ptr<View> child, size_t row, size_t col,
                     size_t rspan = 1, size_t cspan = 1) {
    if (!child) {
      return *this;
    }

    if (!rspan) {
      rspan = 1;
    }
    if (!cspan) {
      cspan = 1;
    }
    childCell[child.get()] = {row, col, rspan, cspan};
    View::addChild(std::move(child));
    return *this;
  }

  GridView &move(View *child, size_t row, size_t col, size_t rspan = 1,
                 size_t cspan = 1) {

    if (!child) {
      return *this;
    }
    if (childCell.contains(child)) {
      if (!rspan) {
        rspan = 1;
      }
      if (!cspan) {
        cspan = 1;
      }
      childCell[child] = {row, col, rspan, cspan};
      markDirty();
    }
    return *this;
  }

  GridView &removeChild(View *child) {

    if (!child) {
      return *this;
    }

    childCell.erase(child);
    View::removeChild(child);
    markDirty();
    return *this;
  }

  GridView &setRowGap(float v) {
    gaps.x = sanitizeFloat(v, 0.0f);
    markDirty();
    return *this;
  }
  GridView &setColumnGap(float v) {
    gaps.y = sanitizeFloat(v, 0.0f);
    markDirty();
    return *this;
  }

  void draw(Renderer &) override { layout(); }

  glm::vec2 getOverflows() {
    update();
    return {colCache.overflow, rowCache.overflow};
  }

private:
  void layout() override {
    if (columns.empty() || rows.empty() || childCell.empty()) {
      colCache = rowCache = {};
      return;
    }

    updateCaches();
    positionChildren();
  }

  void updateCaches() {
    if (!isDirty()) {
      return;
    }
    colCache = computeTrackCache(columns, geometry.z, gaps.y);
    rowCache = computeTrackCache(rows, geometry.w, gaps.x);
    unmarkDirty();
  }

  void positionChildren() {
    for (auto &[child, cell] : childCell) {
      if (!child) {
        continue;
      }

      glm::vec4 rect = {
          colCache.positions[cell.col],                       // X O(1)
          rowCache.positions[cell.row],                       // Y O(1)
          spanSize(colCache, cell.col, cell.colSpan, gaps.y), // W O(S)
          spanSize(rowCache, cell.row, cell.rowSpan, gaps.x)  // H O(S)
      };

      child->setAbsolutePosition(glm::vec2{geometry.x, geometry.y} +
                                 glm::vec2{rect.x, rect.y});
      child->setSize({rect.z, rect.w});
    }
  }

  TrackCache computeTrackCache(const std::vector<Size> &tracks,
                               float constraint, float gap) const {
    TrackCache cache;
    if (tracks.empty())
      return cache;

    float fixed = 0.0f, frTotal = 0.0f;
    for (const auto &t : tracks) {
      if (t.unit == Unit::Px) {
        fixed += t.value;
      } else {
        frTotal += t.value;
      }
    }

    float used = fixed + gap * glm::max(0uz, tracks.size() - 1);
    float remaining = constraint - used;
    float frSize = frTotal > 0.0f ? glm::abs(remaining) / frTotal : 0.0f;

    // 2. Précalcule positions cumulées O(T)
    cache.sizes.reserve(tracks.size());
    cache.positions.reserve(tracks.size());

    float pos = 0.0f;
    for (size_t i = 0; i < tracks.size(); ++i) {
      float size = tracks[i].unit == Unit::Px ? tracks[i].value
                                              : frSize * tracks[i].value;
      cache.sizes.push_back(size);
      cache.positions.push_back(pos);
      pos += size + gap;
    }

    cache.totalSize = pos - gap; // Retire dernier gap
    cache.overflow = glm::max(cache.totalSize - constraint, 0.0f);
    return cache;
  }

  // ⚡ O(1) pour span=1, O(S) pour span>1 (rarement >3)
  float spanSize(const TrackCache &cache, size_t start, size_t span,
                 float gap) const {
    if (span == 1) {
      return cache.sizes[glm::min(start, cache.sizes.size() - 1)];
    }

    float total = 0.0f;
    for (size_t i = 0; i < span; ++i) {
      size_t idx = start + i;
      if (idx < cache.sizes.size()) {
        total += cache.sizes[idx];
        if (i > 0) {
          total += gap;
        }
      }
    }
    return total;
  }

  void invalidateCache() const { colCache = rowCache = {}; }

private:
  std::vector<Size> rows, columns;
  glm::vec2 gaps = {0.0f, 0.0f};
};

} // namespace mka::graphic
