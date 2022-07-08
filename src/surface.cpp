#include "surface.hpp"

#include <vulkan/vulkan_win32.h>

surface::surface(core* engine_core, win32* platform, win32::window* window)
   : engine_core_(engine_core)
   , platform_(platform)
   , platform_window_(window) {
     VkWin32SurfaceCreateInfoKHR create_info {
          .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
          .pNext     = nullptr,
          .flags     = {},
          .hinstance = platform->get_hInstance(),
          .hwnd      = window->get_hWnd(),
     };
     if (auto function = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(vkGetInstanceProcAddr(engine_core_->instance, "vkCreateWin32SurfaceKHR")); function != nullptr)
          function(engine_core_->instance, &create_info, engine_core_->allocator, &surface_);
}

surface::~surface() {
     vkDestroySurfaceKHR(engine_core_->instance, surface_, engine_core_->allocator);
}
