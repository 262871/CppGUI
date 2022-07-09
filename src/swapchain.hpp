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
          
          create_image_views();
     }
     ~swapchain() {
          for (auto& image_view : swapchain_image_views_)
               vkDestroyImageView(device_->logical(), image_view, vulkan_core_->allocator());

          swapchain_image_views_.clear();
          vkDestroySwapchainKHR(device_->logical(), swapchain_, vulkan_core_->allocator());
     }
     
     VkImageView create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels = 1) {
          VkImageViewCreateInfo image_view_create_info {
               .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
               .pNext      = nullptr,
               .flags      = {},
               .image      = image,
               .viewType   = VK_IMAGE_VIEW_TYPE_2D,
               .format     = format,
               .components = {
                  .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                  .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                  .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                  .a = VK_COMPONENT_SWIZZLE_IDENTITY },
               .subresourceRange = { .aspectMask = aspect_flags, .baseMipLevel = 0, .levelCount = mip_levels, .baseArrayLayer = 0, .layerCount = 1 }
          };
          VkImageView image_view {};
          if (vkCreateImageView(device_->logical(), &image_view_create_info, vulkan_core_->allocator(), &image_view) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateImageView failed");
          return image_view;
     }
     
     void create_image_views() {
          swapchain_image_views_.reserve(swapchain_images_.size());
          for (size_t i = 0; i != swapchain_images_.size(); ++i)
               swapchain_image_views_.push_back(create_image_view(swapchain_images_[i], swapchain_format_, VK_IMAGE_ASPECT_COLOR_BIT));
     }

     
  private:
     device*                  device_;
     core*                    vulkan_core_;
     VkSwapchainKHR           swapchain_;
     VkExtent2D               swapchain_extent_;
     std::vector<VkImage>     swapchain_images_;
     VkFormat                 swapchain_format_;
     std::vector<VkImageView> swapchain_image_views_;
};
