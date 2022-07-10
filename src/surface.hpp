#pragma once

#include "Core.hpp"
#include "Win32.hpp"

template <typename T>
class Frame {
  public:
     Frame(T::frame_handle handle) { handle_ = handle; }
     ~Frame() { T::destroyFrame(handle_); }

     T::frame_handle handle() { return handle_; }

  private:
     T::frame_handle handle_;
};

class Surface {
  public:
     Surface(Core* core, Win32* win32, Frame<Win32>* frame)
        : core_(core) {
          VkWin32SurfaceCreateInfoKHR create_info {
               .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
               .pNext     = nullptr,
               .flags     = {},
               .hinstance = win32->instance(),
               .hwnd      = frame->handle(),
          };
          if (auto function = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(vkGetInstanceProcAddr(core_->instance(), "vkCreateWin32SurfaceKHR")); function != nullptr)
               function(core_->instance(), &create_info, core_->allocator(), &surface_);
     }
     ~Surface() {
          vkDestroySurfaceKHR(core_->instance(), surface_, core_->allocator());
     }

     VkSurfaceKHR surfaceKHR() { return surface_; }

  private:
     Core*        core_;
     VkSurfaceKHR surface_;
};
