module;
#include "debug.hpp"
#include <cstddef>
#include <exception>
#include <functional>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

export module mka.graphic.event;
export import mka.graphic.event.definition;
import mka.graphic.log;

namespace mka::graphic {

    using EventHash = size_t;
    using Emitter = void *;
    using Event = std::string_view;
    using LinkID = size_t;

    export class Link {
    private:
        struct Impl {
            Emitter emitter = nullptr;
            EventHash eventHash = 0;
            LinkID index = 0;
        };
        std::shared_ptr<Impl> impl_;

    public:
        Link() : impl_(std::make_shared<Impl>()) {}

        bool valid() const { return impl_ && impl_->emitter != nullptr; }

        // Ami pour EventManager internals
        friend class EventManager;
    };

    class EventManager {
    private:
        using Callback = std::function<void()>;
        using CallbackList = std::vector<Callback>;

        static inline std::unordered_map<Emitter, std::unordered_map<EventHash, CallbackList>>
            callbacks;

    public:
        template <typename Receiver, typename Method>
        static Link link(Emitter emitter, Event event, Receiver *receiver, Method method) {

            if (!emitter) {
                Log::warn("link(...) to {} ignored, emitter is null.", event);
                return Link{};
            }

            static_assert(std::is_member_function_pointer_v<Method>,
                          "Must be member function pointer");

            EventHash evtHash = std::hash<std::string_view>{}(event);
            auto &cb = callbacks[emitter][evtHash];
            LinkID id = cb.size();

            cb.emplace_back([receiver, method, event](auto &&...args) {
                if (receiver) {
                    (receiver->*method)();
                }
            });

            Link result;
            auto impl = std::make_shared<Link::Impl>(emitter, evtHash, id);
            // Direct access via constructor/friend pattern
            result.impl_ = impl;
            return result;
        }

        template <typename F>
        static Link link(Emitter emitter, Event event, std::nullptr_t, F &&lambda) {

            if (!emitter) {
                Log::warn("link(...) to {} ignored, emitter is null.", event);
                return Link{};
            }

            EventHash evtHash = std::hash<std::string_view>{}(event);
            auto &cb = callbacks[emitter][evtHash];
            LinkID id = cb.size();

            cb.emplace_back([lambda = std::forward<F>(lambda), event](auto &&...) { lambda(); });

            Link result;
            result.impl_ = std::make_shared<Link::Impl>(emitter, evtHash, id);
            return result;
        }

        static void unlink(Link &l) {
            if (!l.valid()) {
	            Log::warn("unlink(...) ignored invalid Link object.");
                return;
            }

            if (!l.impl_) {
                return;
            }

            auto emIt = callbacks.find(l.impl_->emitter);
            if (emIt == callbacks.end()) {
                return;
            }

            auto evtIt = emIt->second.find(l.impl_->eventHash);
            if (evtIt != emIt->second.end() && l.impl_->index < evtIt->second.size()) {
                auto &clbk = evtIt->second;
                if (l.impl_->index < clbk.size() - 1) {
                    std::swap(clbk[l.impl_->index], clbk.back());
                }
                clbk.pop_back();
            }

            l.impl_->emitter = nullptr; // Invalidate
        }

        static void unlink(Emitter emitter) {
            if (!emitter) {
                Log::warn("unlink(...) to all events ignored, emitter is null.");
                return;
            }
            callbacks.erase(emitter);
        }

        static void send(Emitter emitter, Event event) {
            if (!emitter) {
                Log::warn("send(...) {} ignored, emitter is null.", event);
                return;
            }

            EventHash evtHash = std::hash<std::string_view>{}(event);
            auto emIt = callbacks.find(emitter);
            if (emIt == callbacks.end()) {
                return;
            }

            auto evtIt = emIt->second.find(evtHash);
            if (evtIt != emIt->second.end()) {
                for (auto &cb : evtIt->second) {
                    try {
                        cb();
                    } catch (const std::exception &e) {
						Log::warn("callback invoke error on {} : {}.", event, e.what());
                    }
                }
            }
        }
    };

    export namespace event {
        inline Link link(Emitter emitter, Event event, std::nullptr_t, auto &&lambda) {
            return EventManager::link(emitter, event, nullptr,
                                      std::forward<decltype(lambda)>(lambda));
        }

        template <typename Receiver, typename Method>
        inline Link link(Emitter emitter, Event event, Receiver *receiver, Method method) {
            return EventManager::link(emitter, event, receiver, method);
        }

        inline void unlink(Link &l) { EventManager::unlink(l); }

        inline void unlink(Emitter emitter) { EventManager::unlink(emitter); }

        inline void send(Emitter emitter, Event event) { EventManager::send(emitter, event); }
    } // namespace event
} // namespace mka::graphic
