#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <volk.h>
#include <vulkan/vulkan_win32.h>

#include <windows.h>

#include <stdexcept>
#include <vector>

class Core {
  public:
     Core();
     ~Core();
     VkAllocationCallbacks* allocator() { return allocator_; }
     VkInstance             instance() { return instance_; }

  private:
     VkAllocationCallbacks*   allocator_ { nullptr };
     VkInstance               instance_;
     std::vector<const char*> extensions_ { "VK_EXT_debug_utils", "VK_KHR_surface", "VK_KHR_win32_surface" };
     std::vector<const char*> layers_ { "VK_LAYER_KHRONOS_validation" };
};

Core::Core() {
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
          .enabledLayerCount       = static_cast<uint32_t>(layers_.size()),
          .ppEnabledLayerNames     = layers_.data(),
          .enabledExtensionCount   = static_cast<uint32_t>(extensions_.size()),
          .ppEnabledExtensionNames = extensions_.data()
     };
     if (vkCreateInstance(&instance_create_info, allocator_, &instance_) != VK_SUCCESS)
          throw std::runtime_error("call to vkCreateInstance failed");

     volkLoadInstance(instance_);
}

Core::~Core() {
     vkDestroyInstance(instance_, nullptr);
}
