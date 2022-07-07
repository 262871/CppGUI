#pragma once

#include <concepts>

template <typename T>
concept singleton = requires {
     {T::get_instance_ref()};
     requires !std::copy_constructible<T> && !std::move_constructible<T>;
};

template <typename T, typename... Args>
concept make_window = requires(T t, Args... args) {
     {t.make_window(args...)} -> std::same_as<typename T::window_handle_type>;
};
template <typename T, typename... Args>
concept destroy_window = requires(T t, Args... args) {
     {t.destroy_window(args...)};
};
template <typename T>
concept windowing = requires(T t) {
     typename T::window_handle_type;
     requires make_window<T, const wchar_t*, void*>;
     requires destroy_window<T, typename T::window_handle_type>;
};

template <typename T>
requires windowing<T> && singleton<T>
class window {
  public:
     template <typename U>
     window(const wchar_t* name, U* pApp) {
          handle_ = T::get_instance_ref().make_window(name, pApp);
     }
     ~window() {
          T::get_instance_ref().destroy_window(handle_);
     }

  private:
     T::window_handle_type handle_ {};
};
