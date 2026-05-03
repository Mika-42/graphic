module;
#include <algorithm>
#include <glm/glm.hpp>
#include <limits>
#include <memory>
#include <ranges>
#include <string>
#include <vector>
#include <glm/gtc/epsilon.hpp>
export module mka.graphic.view;
import mka.graphic.opengl.renderer;
import mka.graphic.sanitize;
import mka.graphic.keyboardview;
import mka.graphic.mouseview;
import mka.graphic.event;
import mka.graphic.log;

bool isEq(const glm::vec2 &a, const glm::vec2 &b, float eps = 0.01f) {
    return glm::all(glm::epsilonEqual(a, b, glm::vec2(eps)));
}

export namespace mka::graphic {
    class View {
    public:
        friend class Layout;
        class Layout {
        public:
			Layout(View& parent) : parentView(parent) {}
            virtual void apply() {	
			};

            virtual ~Layout() = default;
		
			std::vector<std::shared_ptr<View>>& children() { return parentView.children; }
			auto geometry() { return parentView.getGeometry(); } 
		
			auto resolveSize(View& view, const glm::vec2& available) {
				return view.resolveSize(available);
			}

		private:
			View& parentView;
        };

	public:
        enum class SizeMode { Fixed, Auto, Fill };
		void setSizeMode(SizeMode mode) {
			sizeMode = mode;
		}

    public:
        // Constructeur / Destructeur
        View() = default;
        virtual ~View() = default;

    public: // events
        Event<const MouseEventView &> MouseEnter;
        Event<const MouseEventView &> MouseLeave;
        Event<const MouseEventView &> MouseMove;
        Event<> ChildAdded;
        Event<> ChildRemoved;
        Event<const glm::vec2 &> PositionChanged;
        Event<const glm::vec2 &> SizeChanged;
        Event<const glm::vec4 &> RadiusChanged;
        Event<const bool> VisibilityChanged;
        Event<const bool> KeyboardFocused;
        Event<const bool> ClipChanged;
        Event<const glm::vec2 &> OverflowsDetected;
        Event<const glm::vec2 &> ScrollChanged;

    public: // parent/children management
        [[nodiscard]] virtual const View *getParent() const noexcept final { return parent; }

        virtual void addChild(std::shared_ptr<View> child) {
            if (child && child->parent == nullptr) {
                child->parent = this;
                children.emplace_back(child);
                markDirty();

                ChildAdded.send();
            }
        }

        virtual void removeChild(View *child) {
            if (!child) {
                return;
            }
            auto it = std::ranges::find_if(children, [&](const std::shared_ptr<View> &c) { return c.get() == child; });

            if (it != children.end()) {
                (*it)->parent = nullptr;
                (*it)->clipRect.flags.z = NO_CLIP;
                children.erase(it);
                markDirty();

                ChildRemoved.send();
            }
        }

        [[nodiscard]] virtual const std::vector<std::shared_ptr<View>> &getChildren() const noexcept final {
            return children;
        }

    public: // getters
        enum class PositionType : uint8_t { Relative, Absolute };

        glm::vec2 getPosition(PositionType type = PositionType::Relative) {
            updateData();

            if (type == PositionType::Relative) {
                return relativePosition;
            }

            glm::vec2 pos(relativePosition);
            View *p = parent;
            while (p != nullptr) {
                pos += p->relativePosition;
                p = p->parent;
            }

            return pos;
        }

        /*glm::vec2 getSize() { TODO replace by get<Preffered/Min/Max>Size
            updateData();
            return glm::vec2{geometry.z, geometry.w};
        }*/

        const glm::vec2 &getOverflows() {
            computeOverflow();
            return overflows;
        }

        const glm::vec2 &getScroll() const { return scrollOffset; }
        glm::vec4 getClipRadius() const { return clipRect.radius; }
        const int32_t &getClipIndex() const { return currentClipIndex; }
        const glm::vec4 &getGeometry() const { return geometry; }
        const bool &isVisible() const { return visible; }
        const bool &isKeyboardFocused() const { return keyboardFocus; }
        const bool &isMouseFocused() const { return mouseFocus; }
        bool isClip() const { return clipRect.flags.y; }

		const glm::vec2& getPrefferedSize() { return size.preferred; }
    public: // setters
        void setPosition(const glm::vec2 &p) {
            if (!isEq(p, relativePosition)) {
                PositionChanged.send(p);
            }
            relativePosition = p;
            markDirty();
        }

    public:
        void setMinSize(const glm::vec2 &s) {
            size.min.x = sanitizeFloat(s.x);
            size.min.y = sanitizeFloat(s.y);
        }

        void setMaxSize(const glm::vec2 &s) {
            size.max.x = sanitizeFloat(s.x);
            size.max.y = sanitizeFloat(s.y);
        }

        void setPreferredSize(const glm::vec2 &s) {
            size.preferred.x = sanitizeFloat(s.x);
            size.preferred.y = sanitizeFloat(s.y);
        }

	private:
        glm::vec2 resolveSize(const glm::vec2 &available) {
            glm::vec2 s(0.0f);
            glm::vec2 a = glm::max(available, glm::vec2(0.0f));

            switch (sizeMode) {
                case SizeMode::Fixed:
                    s = size.preferred;
                    break;
                case SizeMode::Fill:
                    s = a;
                    break;
                case SizeMode::Auto:
                    s = glm::min(size.preferred, a);
                    break;
            }

            s.x = std::clamp(s.x, size.min.x, size.max.x);
            s.y = std::clamp(s.y, size.min.y, size.max.y);
			return s;
        }


    public:
        void setVisible(bool v) {
            if (visible != v) {
                VisibilityChanged.send(v);
            }

            visible = v;
        }

        void setKeyboardFocus(bool v) {
            if (keyboardFocus != v) {
                KeyboardFocused.send(v);
            }
            keyboardFocus = v;
        }

        void setClip(bool enabled) {
            if (isClip() != enabled) {
                ClipChanged.send(enabled);
            }

            clipRect.flags.y = enabled ? CLIP : 0.0f;
            markDirty();
        }

        void setRadius(const glm::vec4 &radius) {
            if (clipRect.radius != radius) {
                RadiusChanged.send(radius);
            }

            clipRect.radius = radius;
        }

        void setScroll(const glm::vec2 &s) {
            if (scrollOffset != s) {
                ScrollChanged.send(scrollOffset);
            }

            scrollOffset = s;
        }
        void setScrollX(float x) {
            if (scrollOffset.x != x) {
                ScrollChanged.send(scrollOffset);
            }

            scrollOffset.x = x;
        }
        void setScrollY(float y) {
            if (scrollOffset.y != y) {
                ScrollChanged.send(scrollOffset);
            }

            scrollOffset.y = y;
        }

		template<typename T>
		void setLayout() {
			layout = std::make_shared<T>(*this);
		}

        virtual void draw(Renderer &) {} // TODO make it virtual pure

    protected: // updates
        void update(Renderer &renderer) {
            
			if (layout) {
                layout->apply();
            }

            updateGeometry();

            computeOverflow();

            currentClipIndex = parent ? parent->getClipIndex() : NO_CLIP;

            if (isClip()) {
                clipRect.flags.z = currentClipIndex;
                currentClipIndex = renderer.add(clipRect);
            }

            updateChild(renderer);

            draw(renderer);
        }

        std::vector<std::shared_ptr<View>> getSortedChildren() const {
            auto sorted = children;
            std::ranges::stable_sort(sorted, {}, &View::zIndex);
            return sorted;
        }

        void updateChild(Renderer &renderer) {

            for (auto &child : getSortedChildren()) {
                if (child && child->isVisible()) {
                    child->update(renderer);
                }
            }
        }

    public:
        virtual void onKeyboardEvent(const KeyboardEventView & /*keyboard*/) {}

        virtual bool contain(const MouseEventView &mouse) { return clipContain(mouse); }

        virtual bool clipContain(const MouseEventView &mouse) const final {
            return distance(clipRect, mouse.position()) <= mouse.cursorRadius();
        }

        int zIndex = 0;

    protected:
        virtual void updateGeometry() {
            const glm::vec2 apos = getPosition(PositionType::Absolute);
          
			auto parentGeometry = parent->getGeometry();

			auto s = resolveSize({parentGeometry.z, parentGeometry.w});
			geometry.x = apos.x + scrollOffset.x;
            geometry.y = apos.y + scrollOffset.y;
            geometry.z = s.x;
			geometry.w = s.y;

			clipRect.geometry = geometry;
        }

        std::vector<std::shared_ptr<View>> children;
        std::shared_ptr<Layout> layout = std::make_shared<Layout>(*this);

        virtual void markDirty() {
            if (updateDepth == 0)
                dirty = true;
        }

        const bool &isDirty() const { return dirty; }

        void updateData() {
            if (!dirty || updateDepth > 0)
                return;
            updateDepth++;
            updateGeometry();
            updateDepth--;
            dirty = false;
        }

    private:
        void computeOverflow() {
            if (children.empty() || geometry.z <= 0 || geometry.w <= 0) {
                overflows = glm::vec2(0.0f);
                return;
            }

            float minX = std::numeric_limits<float>::max();
            float minY = std::numeric_limits<float>::max();
            float maxX = std::numeric_limits<float>::lowest();
            float maxY = std::numeric_limits<float>::lowest();

            for (const auto &child : children) {
                if (child->geometry.z <= 0 || child->geometry.w <= 0)
                    continue;

                minX = glm::min(minX, child->geometry.x);
                minY = glm::min(minY, child->geometry.y);
                maxX = glm::max(maxX, child->geometry.x + child->geometry.z);
                maxY = glm::max(maxY, child->geometry.y + child->geometry.w);
            }

            if (maxX <= minX) {
                overflows = glm::vec2(0.0f);
                return;
            }

            const float contRight = geometry.x + geometry.z;
            const float contBottom = geometry.y + geometry.w;

            // Overflow total (gauche + droite, haut + bas)
            overflows = {std::max(0.0f, minX - geometry.x) + std::max(0.0f, maxX - contRight),
                         std::max(0.0f, minY - geometry.y) + std::max(0.0f, maxY - contBottom)};

            if (overflows.x > 0.0f || overflows.y > 0.0f) {
                OverflowsDetected.send(overflows);
            }
        }

    private:
        View *parent = nullptr;
        Rectangle clipRect = {};

        bool visible = true;
        bool keyboardFocus = false;
        bool mouseFocus = false;
        bool dirty = true;

        int32_t updateDepth = 0;
        int32_t currentClipIndex = NO_CLIP;

        struct Size {
            glm::vec2 min{0.0f};
            glm::vec2 preferred{0.0f};
            glm::vec2 max{std::numeric_limits<float>::max()};
        } size = {};

        SizeMode sizeMode = SizeMode::Fixed;

        glm::vec4 geometry = glm::vec4(0.0f);
        glm::vec2 relativePosition = glm::vec2(0.0f);
        glm::vec2 overflows = glm::vec2(0.0f);
        glm::vec2 scrollOffset = glm::vec2(0.0f);
    };

} // namespace mka::graphic
