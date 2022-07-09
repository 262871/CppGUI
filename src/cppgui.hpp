#pragma once

#include "core.hpp"
#include "debug_messenger.hpp"
#include "device.hpp"
#include "surface.hpp"
#include "win32.hpp"
#include "swapchain.hpp"

#include <memory>
#include <vector>

class wengine {
     
};

class window {
  public:
     window(const wchar_t* window_name, win32* platform, core* vulkan_core)
        : window_name_(window_name)
        , event_sys_()
        , frame_(platform->create_frame(window_name, &event_sys_))
        , surface_(vulkan_core, platform, &frame_) {
          close_subscription_ = subscribe_on_close([this] { event_sys_.should_close = true; });
     }

     std::unique_ptr<event_dispatcher<std::function<void()>>::subscription> subscribe_on_close(std::function<void(void)> callback) {
          return event_sys_.close_dispatcher.subscribe(callback);
     }

     bool should_close() const { return event_sys_.should_close; }
     auto get_surface() { return &surface_; }

  private:
     const wchar_t* window_name_;
     event_system   event_sys_;
     frame<win32>   frame_;
     surface        surface_;

     std::unique_ptr<event_dispatcher<std::function<void()>>::subscription> close_subscription_;
};

class cppgui {
  public:
     cppgui()
        : platform_()
        , engine_core_()
        , debug_(&engine_core_) {}
     ~cppgui() {
          shutdown();
     }
     void shutdown() {
          swapchain_.reset();
     }
     void initialize() {
          auto name = L"Vulkan and win32";
          windows_.emplace_back(std::make_unique<window>(name, &platform_, &engine_core_));
          device_                   = std::make_unique<device>(&engine_core_);
          swapchain_                = std::make_unique<swapchain>(windows_[0].get()->get_surface(), device_.get(), &engine_core_);
          swapchain_lifetime_guard_ = windows_[0].get()->subscribe_on_close([this] { swapchain_.reset(); });
     }
     void run() {
          initialize();
          while (!windows_.empty()) {
               platform_.process_messages();
               std::erase_if(windows_, [](const auto& w) { return w->should_close(); });
          }
     }

  private:
     win32 platform_;
     core  engine_core_;

     debug_messenger debug_;

     std::vector<std::unique_ptr<window>> windows_ {};
     std::unique_ptr<device>              device_;
     std::unique_ptr<swapchain>           swapchain_;

     std::unique_ptr<event_dispatcher<std::function<void()>>::subscription> swapchain_lifetime_guard_;
};
