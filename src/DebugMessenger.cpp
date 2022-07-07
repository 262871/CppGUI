#include "DebugMessenger.hpp"

DebugMessenger::DebugMessenger(VkInstance instance)
   : instance_(instance) {
     VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info {
          .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
          .pNext           = nullptr,
          .flags           = {},
          .messageSeverity = { VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT },
          .messageType     = { VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT },
          .pfnUserCallback = callback,
          .pUserData       = nullptr
     };

     if (vkCreateDebugUtilsMessengerEXT(instance_, &debug_messenger_info, nullptr, &debug_messenger_) != VK_SUCCESS)
          throw std::runtime_error("call to vkCreateDebugUtilsMessengerEXT failed");
}

DebugMessenger::~DebugMessenger() {
     vkDestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessenger::callback(VkDebugUtilsMessageSeverityFlagBitsEXT level, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* warn, void*) {
     if (level > VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
          fmt::print(stderr, "Error: {}\n", warn->pMessage);
     else if (level != VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
          fmt::print(stdout, "Varning: {}\n", warn->pMessage);
     else
          fmt::print(stdout, "Info: {}\n", warn->pMessage);
     return VK_FALSE;
}