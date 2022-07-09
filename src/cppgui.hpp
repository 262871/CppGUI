#pragma once

#include "core.hpp"
#include "debug_messenger.hpp"
#include "device.hpp"
#include "render_pass.hpp"
#include "surface.hpp"
#include "swapchain.hpp"
#include "win32.hpp"

#include <memory>
#include <vector>

class window {
  public:
     window(const wchar_t* window_name, win32* platform, core* vulkan_core)
        : window_name_(window_name)
        , event_system_()
        , frame_(platform->create_frame(window_name, &event_system_))
        , surface_(vulkan_core, platform, &frame_)
        , device_(vulkan_core)
        , swapchain_(swapchain::make(&surface_, &device_, vulkan_core))
        , render_pass_(&device_, vulkan_core, &swapchain_) {
          close_subscription_ = subscribe_on_close([this] { should_close_ = true; });
     }

     std::unique_ptr<event_dispatcher<std::function<void()>>::subscription> subscribe_on_close(std::function<void(void)> callback) {
          return event_system_.close_dispatcher.subscribe(callback);
     }

     bool should_close() const { return should_close_; }
     auto get_surface() { return &surface_; }

  private:
     const wchar_t* window_name_;
     event_system   event_system_;
     bool           should_close_ { false };
     frame<win32>   frame_;
     surface        surface_;
     device         device_;
     swapchain      swapchain_;
     render_pass    render_pass_;

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
     }
     void initialize() {
          auto name = L"Vulkan and win32";
          windows_.emplace_back(std::make_unique<window>(name, &platform_, &engine_core_));
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
};
