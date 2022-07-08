#pragma once

#include "core.hpp"
#include "win32.hpp"

#include <vulkan/vulkan_win32.h>

template <typename T>
class frame {
  public:
     frame(T::frame_handle handle) { handle_ = handle; }
     ~frame() { T::destroy_frame(handle_); }

     T::frame_handle handle() { return handle_; }

  private:
     T::frame_handle handle_;
};

class surface {
  public:
     surface(core* vulkan_core, win32* platform, frame<win32>* frame)
        : vulkan_core_(vulkan_core) {
          VkWin32SurfaceCreateInfoKHR create_info {
               .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
               .pNext     = nullptr,
               .flags     = {},
               .hinstance = platform->instance(),
               .hwnd      = frame->handle(),
          };
          if (auto function = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(vkGetInstanceProcAddr(vulkan_core_->instance(), "vkCreateWin32SurfaceKHR")); function != nullptr)
               function(vulkan_core_->instance(), &create_info, vulkan_core_->allocator(), &surface_);
     }
     ~surface() {
          vkDestroySurfaceKHR(vulkan_core_->instance(), surface_, vulkan_core_->allocator());
     }

  private:
     core*        vulkan_core_;
     VkSurfaceKHR surface_;
};
