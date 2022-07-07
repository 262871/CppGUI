#include "instance.hpp"

instance::instance() {
     if (volkInitialize() != VK_SUCCESS)
          throw std::runtime_error("call to volkInitialize failed");

     uint32_t version {};
     vkEnumerateInstanceVersion(&version);

     VkApplicationInfo application_info {
          .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
          .pNext              = nullptr,
          .pApplicationName   = "GUI",
          .applicationVersion = 1,
          .pEngineName        = "CppGUI",
          .engineVersion      = 1,
          .apiVersion         = version
     };
     auto layers = "VK_LAYER_KHRONOS_validation";

     auto                 debug_ext { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
     VkInstanceCreateInfo instance_create_info {
          .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
          .pNext                   = nullptr,
          .flags                   = {},
          .pApplicationInfo        = &application_info,
          .enabledLayerCount       = 1,
          .ppEnabledLayerNames     = &layers,
          .enabledExtensionCount   = 1,
          .ppEnabledExtensionNames = &debug_ext
     };
     if (vkCreateInstance(&instance_create_info, nullptr, &instance_) != VK_SUCCESS)
          throw std::runtime_error("call to vkCreateInstance failed");

     volkLoadInstance(instance_);
}

instance::~instance() {
     vkDestroyInstance(instance_, nullptr);
}

VkInstance instance::get() {
     return instance_;
}
