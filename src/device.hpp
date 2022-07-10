#pragma once

#include "Core.hpp"

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include <stdexcept>
#include <vector>

class Device {
  public:
     Device(Core* core);
     ~Device();

     auto physical() { return physicalDevice_; }
     auto logical() { return device_; }

     auto present() { return presentQueue_; }
     auto graphics() { return graphicsQueue_; }
     auto transfer() { return transferQueue_; }

  private:
     void queueSetup() {
          vkGetDeviceQueue(device_, 0, 0, &presentQueue_);
          vkGetDeviceQueue(device_, 0, 1, &graphicsQueue_);
          vkGetDeviceQueue(device_, 1, 0, &transferQueue_);
     }

     Core*            core_;
     VkPhysicalDevice physicalDevice_;
     VkDevice         device_;

     VkQueue presentQueue_;
     VkQueue graphicsQueue_;
     VkQueue transferQueue_;

     std::vector<const char*> extensions_ { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};

Device::Device(Core* core)
   : core_(core) {
     uint32_t count = 1;
     vkEnumeratePhysicalDevices(core_->instance(), &count, &physicalDevice_);

     count    = 2;
     auto qfp = new VkQueueFamilyProperties[2];
     vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &count, qfp);

     fmt::print("vkGetPhysicalDeviceQueueFamilyProperties count: {}\n", count);
     fmt::print("QueueFamilyProperties[0].queueCount: {}\n", qfp[0].queueCount);
     fmt::print("QueueFamilyProperties[1].queueCount: {}\n", qfp[1].queueCount);
     delete[] qfp;

     const std::vector<float>                   queue_priorities { 1.f, 1.f };
     const std::vector<VkDeviceQueueCreateInfo> queue_create_infos = {
          VkDeviceQueueCreateInfo {
             .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
             .pNext            = nullptr,
             .flags            = {},
             .queueFamilyIndex = 0,
             .queueCount       = 2,
             .pQueuePriorities = &queue_priorities[0] },
          VkDeviceQueueCreateInfo {
             .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
             .pNext            = nullptr,
             .flags            = {},
             .queueFamilyIndex = 1,
             .queueCount       = 1,
             .pQueuePriorities = &queue_priorities[1] }
     };

     VkPhysicalDeviceFeatures device_features {};
     device_features.samplerAnisotropy = VK_TRUE;
     device_features.sampleRateShading = VK_TRUE;

     VkDeviceCreateInfo device_create_info {
          .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
          .pNext                   = nullptr,
          .flags                   = {},
          .queueCreateInfoCount    = static_cast<uint32_t>(queue_create_infos.size()),
          .pQueueCreateInfos       = queue_create_infos.data(),
          .enabledLayerCount       = 0,
          .ppEnabledLayerNames     = nullptr,
          .enabledExtensionCount   = static_cast<uint32_t>(extensions_.size()),
          .ppEnabledExtensionNames = extensions_.data(),
          .pEnabledFeatures        = &device_features
     };
     if (vkCreateDevice(physicalDevice_, &device_create_info, core_->allocator(), &device_) != VK_SUCCESS)
          throw std::runtime_error("call to vkCreateDevice failed");

     queueSetup();
}

Device::~Device() {
     vkDestroyDevice(device_, core_->allocator());
}
