module;

#include <vector>
export module mka.graphic.renderlist;
import mka.graphic.view;

export namespace mka::graphic {

struct RenderItem {
  View *view;
  std::vector<int> zPath;
};

void collect(mka::graphic::View *view, std::vector<int> &path,
             std::vector<RenderItem> &out) {

  if (!view->isVisible()) {
    return;
  }

  path.push_back(view->zIndex);

  out.push_back({view, path});

  for (auto &child : view->getChildren()) {
    collect(child.get(), path, out);
  }

  path.pop_back();
}
} // namespace mka::graphic
