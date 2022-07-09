#pragma once

#include "core.hpp"

#include <volk.h>

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include <stdexcept>
#include <vector>

class device {
  public:
     device(core* engine_core);
     ~device();

     auto physical() { return physical_device_; }
     auto logical() { return device_; }

     auto present() { return present_queue_; }
     auto graphics() { return graphics_queue_; }
     auto transfer() { return transfer_queue_; }

  private:
     void queue_setup() {
          vkGetDeviceQueue(device_, 0, 0, &present_queue_);
          vkGetDeviceQueue(device_, 0, 1, &graphics_queue_);
          vkGetDeviceQueue(device_, 1, 0, &transfer_queue_);
     }

     core*            engine_core_;
     VkPhysicalDevice physical_device_;
     VkDevice         device_;

     VkQueue present_queue_;
     VkQueue graphics_queue_;
     VkQueue transfer_queue_;

     std::vector<const char*> extensions_ { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};

device::device(core* engine_core)
   : engine_core_(engine_core) {
     uint32_t count = 1;
     vkEnumeratePhysicalDevices(engine_core_->instance(), &count, &physical_device_);

     count    = 2;
     auto qfp = new VkQueueFamilyProperties[2];
     vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &count, qfp);

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
     if (vkCreateDevice(physical_device_, &device_create_info, engine_core_->allocator(), &device_) != VK_SUCCESS)
          throw std::runtime_error("call to vkCreateDevice failed");

     queue_setup();
}

device::~device() {
     vkDestroyDevice(device_, engine_core_->allocator());
}
