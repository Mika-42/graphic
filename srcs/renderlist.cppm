module;
#include <cstdint>
#include <vector>
export module mka.graphic.renderlist;
import mka.graphic.view;

export namespace mka::graphic {

struct RenderItem {
  View *view;
  uint64_t zPath64; // [lvl3 | lvl2 | lvl1 | lvl0] 16-bit chacun

  RenderItem() = default;
  RenderItem(View *v, uint64_t z) : view(v), zPath64(z) {}
};

void collect(mka::graphic::View *view, std::vector<RenderItem> &out,
             uint64_t parentZ = 0, uint8_t depth = 0) {

  if (depth >= 4) return;

  uint64_t zPath64 = (parentZ << 16) | static_cast<uint32_t>(view->zIndex);

  out.emplace_back(view, zPath64);

  for (auto &child : view->getChildren()) {
    collect(child.get(), out, static_cast<uint32_t>(view->zIndex), depth + 1);
  }
}
} // namespace mka::graphic
