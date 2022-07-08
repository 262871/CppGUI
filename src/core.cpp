#include "core.hpp"

#include <stdexcept>

core::core(std::vector<const char*> const& extensions, std::vector<const char*> const& layers) {
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

     VkInstanceCreateInfo instance_create_info {
          .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
          .pNext                   = nullptr,
          .flags                   = {},
          .pApplicationInfo        = &application_info,
          .enabledLayerCount       = static_cast<uint32_t>(layers.size()),
          .ppEnabledLayerNames     = layers.data(),
          .enabledExtensionCount   = static_cast<uint32_t>(extensions.size()),
          .ppEnabledExtensionNames = extensions.data()
     };
     if (vkCreateInstance(&instance_create_info, allocator, &instance) != VK_SUCCESS)
          throw std::runtime_error("call to vkCreateInstance failed");

     volkLoadInstance(instance);
}

core::~core() {
     vkDestroyInstance(instance, nullptr);
}
