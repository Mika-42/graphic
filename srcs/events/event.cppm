module;
#include <cstddef>
#include <exception>
#include <functional>
#include <memory>
#include <vector>
#include <forward_list>

export module mka.graphic.event;
import mka.graphic.log;

namespace mka::graphic {

    export template <typename... Args> class Event {
    public:
        typedef std::shared_ptr<std::function<void(Args...)>> Link;
        Event() = default;

        void send(const Args &...args) {
            bool cleanup = false;
            for (const auto &lnk_weak : links) {

                if (const auto &lnk_function = lnk_weak.lock()) {
                    (*lnk_function)(args...);
                } else {
                    // weak pointer expired, do a cleanup round
                    cleanup = true;
                }
            }

            if (cleanup) {
                links.remove_if([](auto lnk) { return lnk.expired(); });
            }
        }

        Link link(std::function<void(Args...)> fn) {
            auto lnk = std::make_shared<std::function<void(Args...)>>(fn);
            links.emplace_front(lnk);
            return lnk;
        }

        template <typename T> Link link(T *instance, void (T::*method)(Args...)) {
            return link([=](Args... args) { (instance->*method)(args...); });
        }

        void unlink(Link lnk) {
            links.remove_if([&](const auto &lnk_weak) {
                auto sp = lnk_weak.lock();
                return sp && sp == lnk;
            });
        }

    private:
        std::forward_list<std::weak_ptr<std::function<void(Args...)>>> links;
    };

} // namespace mka::graphic

export namespace mka::graphic::event {

// LINK (emitter → receiver)
template <typename EmitterClass, typename OwnerClass, typename ReceiverClass, typename... EventArgs>
auto link(
    EmitterClass& sender,
    Event<EventArgs...> OwnerClass::*event_ptr,
    ReceiverClass& receiver,
    void (ReceiverClass::*method)(EventArgs...)) {
    auto& event_ref = sender.*event_ptr;

    return event_ref.link(
        [&sender, &receiver, method](EventArgs... args) {
            (receiver.*method)(args...);
        }
    );
}

template <typename EmitterClass, typename OwnerClass, typename Fn, typename... EventArgs>
auto link(EmitterClass& sender, Event<EventArgs...> OwnerClass::*event_ptr, Fn&& fn) {
    return (sender.*event_ptr).link(std::forward<Fn>(fn));
}

// SEND
template <typename EmitterClass, typename... EventArgs>
void send(
    EmitterClass& sender,
    Event<EventArgs...> EmitterClass::*event_ptr,
    EventArgs... args) {
    (sender.*event_ptr).send(args...);
}

// UNLINK
template <typename EmitterClass, typename... EventArgs>
void unlink(
    EmitterClass& sender,
    Event<EventArgs...> EmitterClass::*event_ptr,
    typename Event<EventArgs...>::Link lnk) {
    (sender.*event_ptr).unlink(lnk);
}

} // namespace mka::graphic::event
