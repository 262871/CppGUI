#pragma once

#include "core.hpp"

#include <volk.h>

class debug_messenger {
  public:
     debug_messenger(core*);
     ~debug_messenger();

  private:
     static VKAPI_ATTR VkBool32 VKAPI_CALL callback(VkDebugUtilsMessageSeverityFlagBitsEXT level, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* warn, void*);

     core*                    engine_core_;
     VkDebugUtilsMessengerEXT debug_messenger_;
};
