#pragma once

#include <concepts>
#include <functional>
#include <map>
#include <mutex>
#include <unordered_map>


template <typename F, typename... ArgT>
requires std::invocable<F, ArgT...>
class event_dispatcher {
  public:
     class subscription {
       public:
          subscription(event_dispatcher* source, const F& listener)
             : source_(source) { source_->add_listener(this, listener); }
          ~subscription() {
               if (source_ != nullptr)
                    source_->remove_listener(this);
          }
          subscription(const subscription&)            = delete;
          subscription(subscription&&)                 = delete;
          subscription& operator=(const subscription&) = delete;
          subscription& operator=(subscription&&)      = delete;

          bool is_valid() { return source_ != nullptr; }

       private:
          event_dispatcher* source_;
          friend event_dispatcher;
          void invalidate() { source_ = nullptr; }
     };
     ~event_dispatcher() {
          std::unique_lock lock(subscribers_mutex_);
          for (const auto& [subscription, invocable] : subscribers_)
               subscription->invalidate();
     }
     subscription subscribe(const F& listener) { return subscription(this, listener); }

     void signal(ArgT... arguments) {
          std::unique_lock lock(subscribers_mutex_);
          for (const auto& [subscription, invocable] : subscribers_)
               invocable(arguments...);
     }

  private:
     std::unordered_map<subscription*, F> subscribers_ {};
     std::mutex                           subscribers_mutex_ {};

     void add_listener(subscription* subscriber, const F& listener) {
          std::unique_lock lock(subscribers_mutex_);
          subscribers_[subscriber] = listener;
     }
     void remove_listener(subscription* subscriber) {
          std::unique_lock lock(subscribers_mutex_);
          subscribers_.erase(subscriber);
     }
};

class event_system {
  public:
     bool should_close {false};
     // event_dispatcher<std::function<void(void)>> close_dispatcher{};
};
