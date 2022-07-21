#pragma once

#include "Core.hpp"


#include <stdexcept>
#include <vector>

class Device {
  public:
     enum QueuePriority {
          GRAPHICS_LOW,
          GRAPHICS_MEDIUM,
          GRAPHICS_HIGH,
          GRAPHICS_REALTIME,
          TRANSFER_LOW,
          TRANSFER_MEDIUM,
          TRANSFER_HIGH,
          TRANSFER_REALTIME,
          COMPUTE_LOW,
          COMPUTE_MEDIUM,
          COMPUTE_HIGH,
          COMPUTE_REALTIME
     };

     Device(Core* core);
     ~Device();

     auto physical() { return physicalDevice_; }
     auto logical() { return device_; }

     auto present() { return queue_; }
     auto graphics() { return queue_; }
     auto transfer() { return queue_; }

  private:
     void queueSetup() {
          vkGetDeviceQueue(device_, 0, 0, &queue_);
     }

     Core*            core_;
     VkPhysicalDevice physicalDevice_;
     VkDevice         device_;

     VkQueue queue_;

     std::vector<const char*> extensions_ { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};

Device::Device(Core* core)
   : core_(core) {
     uint32_t count = 1;
     vkEnumeratePhysicalDevices(core_->instance(), &count, &physicalDevice_);

     count = 2;
     VkQueueFamilyProperties qfp[2];
     vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &count, qfp);

     fmt::print("vkGetPhysicalDeviceQueueFamilyProperties count: {}\n", count);
     fmt::print("QueueFamilyProperties[0].queueCount: {}\n", qfp[0].queueCount);
     fmt::print("QueueFamilyProperties[1].queueCount: {}\n", qfp[1].queueCount);

     float queuePriority = 1.f;

     const std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos = {
          VkDeviceQueueCreateInfo {
             .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
             .pNext            = nullptr,
             .flags            = {},
             .queueFamilyIndex = 0,
             .queueCount       = 1,
             .pQueuePriorities = &queuePriority }
     };

     VkPhysicalDeviceFeatures physicalDeviceFeatures {};
     physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;
     physicalDeviceFeatures.sampleRateShading = VK_TRUE;
     physicalDeviceFeatures.fillModeNonSolid = VK_TRUE;

     VkDeviceCreateInfo device_create_info {
          .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
          .pNext                   = nullptr,
          .flags                   = {},
          .queueCreateInfoCount    = static_cast<uint32_t>(deviceQueueCreateInfos.size()),
          .pQueueCreateInfos       = deviceQueueCreateInfos.data(),
          .enabledLayerCount       = 0,
          .ppEnabledLayerNames     = nullptr,
          .enabledExtensionCount   = static_cast<uint32_t>(extensions_.size()),
          .ppEnabledExtensionNames = extensions_.data(),
          .pEnabledFeatures        = &physicalDeviceFeatures
     };
     if (vkCreateDevice(physicalDevice_, &device_create_info, core_->allocator(), &device_) != VK_SUCCESS)
          throw std::runtime_error("call to vkCreateDevice failed");

     queueSetup();
}

Device::~Device() {
     vkDestroyDevice(device_, core_->allocator());
}
