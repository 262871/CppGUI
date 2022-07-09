#pragma once

#include "core.hpp"
#include "device.hpp"
#include "surface.hpp"

#include <vector>
class swapchain {
  public:
     swapchain(surface* window, device* gpu, core* vulkan_core)
        : device_(gpu)
        , vulkan_core_(vulkan_core) {
          VkSurfaceCapabilitiesKHR capabilities;
          if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu->physical(), window->surfaceKHR(), &capabilities) != VK_SUCCESS)
               throw std::runtime_error("call to vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed");

          fmt::print("capabilities.currentExtent: {}, {}\n", capabilities.currentExtent.width, capabilities.currentExtent.height);
          swapchain_extent_ = capabilities.currentExtent;
          swapchain_format_ = VK_FORMAT_B8G8R8A8_SRGB;

          VkSwapchainCreateInfoKHR swapchain_create_info {
               .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
               .pNext                 = nullptr,
               .flags                 = {},
               .surface               = window->surfaceKHR(),
               .minImageCount         = 2,
               .imageFormat           = swapchain_format_,
               .imageColorSpace       = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
               .imageExtent           = swapchain_extent_,
               .imageArrayLayers      = 1,
               .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
               .imageSharingMode      = VK_SHARING_MODE_CONCURRENT,
               .queueFamilyIndexCount = static_cast<uint32_t>(device_->queue_family_indices().size()),
               .pQueueFamilyIndices   = device_->queue_family_indices().data(),
               .preTransform          = capabilities.currentTransform,
               .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
               .presentMode           = VK_PRESENT_MODE_MAILBOX_KHR,
               .clipped               = VK_TRUE,
               .oldSwapchain          = nullptr
          };

          if (vkCreateSwapchainKHR(device_->logical(), &swapchain_create_info, vulkan_core_->allocator(), &swapchain_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateSwapchainKHR failed");

          uint32_t count {};
          if (vkGetSwapchainImagesKHR(device_->logical(), swapchain_, &count, nullptr) != VK_SUCCESS)
               throw std::runtime_error("call to vkGetSwapchainImagesKHR failed");

          swapchain_images_.resize(count);
          if (vkGetSwapchainImagesKHR(device_->logical(), swapchain_, &count, swapchain_images_.data()) != VK_SUCCESS)
               throw std::runtime_error("call to vkGetSwapchainImagesKHR failed");
     }
     ~swapchain() {
          vkDestroySwapchainKHR(device_->logical(), swapchain_, vulkan_core_->allocator());
     }

  private:
     device*              device_;
     core*                vulkan_core_;
     VkSwapchainKHR       swapchain_;
     VkExtent2D           swapchain_extent_;
     std::vector<VkImage> swapchain_images_;
     VkFormat             swapchain_format_;
};
