#pragma once

#include "core.hpp"

#include <volk.h>

#define FMT_HEADER_ONLY
#include <fmt/color.h>
#include <fmt/format.h>

#include <cstring>
#include <stdexcept>

class debug_messenger {
  public:
     debug_messenger(core* vulkan_core)
        : vulkan_core_(vulkan_core) {
          VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info {
               .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
               .pNext           = nullptr,
               .flags           = {},
               .messageSeverity = { VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT },
               .messageType     = { VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT },
               .pfnUserCallback = callback,
               .pUserData       = nullptr
          };

          if (vkCreateDebugUtilsMessengerEXT(vulkan_core_->instance(), &debug_messenger_info, vulkan_core_->allocator(), &debug_messenger_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateDebugUtilsMessengerEXT failed");
     }
     ~debug_messenger() { vkDestroyDebugUtilsMessengerEXT(vulkan_core_->instance(), debug_messenger_, vulkan_core_->allocator()); }

  private:
     static VKAPI_ATTR VkBool32 VKAPI_CALL callback(VkDebugUtilsMessageSeverityFlagBitsEXT level, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* warn, void*);

     core*                    vulkan_core_;
     VkDebugUtilsMessengerEXT debug_messenger_;
};

VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger::callback(VkDebugUtilsMessageSeverityFlagBitsEXT level, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* warn, void*) {
     if (level > VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
          fmt::print(stderr, fg(fmt::color::turquoise), "{}\n", warn->pMessage);
     else if (level > VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
          fmt::print(stdout, fmt::emphasis::blink | fg(fmt::color::gold), "{}\n", warn->pMessage);
     else if (level != VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
          fmt::print(stdout, fg(fmt::color::light_blue), "{}\n", warn->pMessage);
     else {
          if (std::strncmp("Device Extension:", warn->pMessage, 16) != 0)
               fmt::print(stdout, fg(fmt::color::dim_gray), "{}\n", warn->pMessage);
          // else
          //      fmt::print(stdout, fmt::emphasis::faint | fg(fmt::color::dim_gray), "{}\n", warn->pMessage);
     }
     return VK_FALSE;
}
