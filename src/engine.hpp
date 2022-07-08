#pragma once

#include "core.hpp"
#include "debug_messenger.hpp"
#include "device.hpp"
#include "renderer.hpp"

// Can't expand VK_KHR_WIN32_SURFACE_EXTENSION_NAME and remove include. TODO: figure out why
#include <vulkan/vulkan_win32.h>

class engine {
  public:
     engine(win32* platform, win32::window* window);
     ~engine();

  private:
     std::vector<const char*> extensions_ { VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
     std::vector<const char*> layers_ { "VK_LAYER_KHRONOS_validation" };
     core                     core_;
     debug_messenger          messenger_;
     renderer                 renderer_;
     device                   device_;
};
