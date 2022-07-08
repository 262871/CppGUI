#include "device.hpp"

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include <stdexcept>
#include <vector>

device::device(core* engine_core)
   : engine_core_(engine_core) {
     uint32_t count = 1;
     vkEnumeratePhysicalDevices(engine_core_->instance, &count, &physical_device_);

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
             .queueCount       = 1,
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

     std::vector<const char*> extensions_ { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
     std::vector<const char*> layers_ { "VK_LAYER_KHRONOS_validation" };

     VkDeviceCreateInfo device_create_info {
          .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
          .pNext                   = nullptr,
          .flags                   = {},
          .queueCreateInfoCount    = static_cast<uint32_t>(queue_create_infos.size()),
          .pQueueCreateInfos       = queue_create_infos.data(),
          .enabledLayerCount       = static_cast<uint32_t>(layers_.size()),
          .ppEnabledLayerNames     = layers_.data(),
          .enabledExtensionCount   = static_cast<uint32_t>(extensions_.size()),
          .ppEnabledExtensionNames = extensions_.data(),
          .pEnabledFeatures        = &device_features
     };
     if (vkCreateDevice(physical_device_, &device_create_info, nullptr, &device_) != VK_SUCCESS)
          throw std::runtime_error("call to vkCreateDevice failed");
}

device::~device() {
     vkDestroyDevice(device_, engine_core_->allocator);
}
