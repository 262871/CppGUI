#pragma once

#include <volk.h>

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include <stdexcept>

class DebugMessenger {
  public:
     DebugMessenger(VkInstance instance);
     ~DebugMessenger();
     
  private:
     static VKAPI_ATTR VkBool32 VKAPI_CALL callback(VkDebugUtilsMessageSeverityFlagBitsEXT level, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* warn, void*);
     
     VkInstance instance_ {};
     VkDebugUtilsMessengerEXT debug_messenger_ {};
};
