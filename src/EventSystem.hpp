#pragma once

#include <concepts>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

template <typename F, typename... ArgT>
requires std::invocable<F, ArgT...>
class EventDispatcher {
  public:
     class Subscription {
       public:
          Subscription(EventDispatcher* source, const F& listener)
             : source_(source) { source_->add_listener(this, listener); }
          ~Subscription() {
               if (source_ != nullptr)
                    source_->remove_listener(this);
          }
          Subscription(const Subscription&)            = delete;
          Subscription(Subscription&&)                 = delete;
          Subscription& operator=(const Subscription&) = delete;
          Subscription& operator=(Subscription&&)      = delete;

          bool is_valid() { return source_ != nullptr; }

       private:
          EventDispatcher* source_;
          friend EventDispatcher;
          void invalidate() { source_ = nullptr; }
     };
     EventDispatcher()                                  = default;
     EventDispatcher(const EventDispatcher&)            = delete;
     EventDispatcher(EventDispatcher&&)                 = delete;
     EventDispatcher& operator=(const EventDispatcher&) = delete;
     EventDispatcher& operator=(EventDispatcher&&)      = delete;
     ~EventDispatcher() {
          std::unique_lock lock(subscribers_mutex_);
          for (const auto& [Subscription, invocable] : subscribers_)
               Subscription->invalidate();
     }
     std::unique_ptr<Subscription> subscribe(const F& listener) { return std::make_unique<Subscription>(this, listener); }

     void signal(ArgT... arguments) {
          std::unique_lock lock(subscribers_mutex_);
          for (const auto& [Subscription, invocable] : subscribers_)
               invocable(arguments...);
     }

  private:
     std::unordered_map<Subscription*, F> subscribers_ {};
     std::mutex                           subscribers_mutex_ {};

     void add_listener(Subscription* subscriber, const F& listener) {
          std::unique_lock lock(subscribers_mutex_);
          subscribers_[subscriber] = listener;
     }
     void remove_listener(Subscription* subscriber) {
          std::unique_lock lock(subscribers_mutex_);
          subscribers_.erase(subscriber);
     }
};

class EventSystem {
  public:
     EventDispatcher<std::function<void(void)>> closeDispatcher {};
};
