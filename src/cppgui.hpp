#pragma once

#include "core.hpp"
#include "debug_messenger.hpp"
#include "device.hpp"
#include "surface.hpp"
#include "win32.hpp"

#include <memory>
#include <vector>

class window {
  public:
     window(const wchar_t* window_name, win32* platform, core* vulkan_core)
        : window_name_(window_name)
        , event_sys_()
        , frame_(platform->create_frame(window_name, &event_sys_))
        , surface_(vulkan_core, platform, &frame_) {
     }

     bool should_close() const { return event_sys_.should_close; }

  private:
     const wchar_t* window_name_;
     event_system   event_sys_;
     frame<win32>   frame_;
     surface        surface_;
};

class cppgui {
  public:
     cppgui()
        : platform_()
        , engine_core_()
        , debug_(&engine_core_) {}
     void initialize() {
          auto name = L"Vulkan and win32";
          windows_.emplace_back(name, &platform_, &engine_core_);
          device_ = std::make_unique<device>(&engine_core_);
     }
     void run() {
          initialize();
          while (!windows_.empty()) {
               platform_.process_messages();
               std::erase_if(windows_, [](const window& w) { return w.should_close(); });
          }
     }

  private:
     win32 platform_;
     core  engine_core_;

     debug_messenger debug_;

     std::vector<window>     windows_ {};
     std::unique_ptr<device> device_;
};
