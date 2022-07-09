#pragma once

#include "core.hpp"
#include "device.hpp"
#include "image_resource2D.hpp"
#include "surface.hpp"

#include <optional>
#include <vector>

class swapchain {
  public:
     static swapchain make(surface* window, device* gpu, core* vulkan_core) {
          VkSurfaceCapabilitiesKHR capabilities;
          if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu->physical(), window->surfaceKHR(), &capabilities) != VK_SUCCESS)
               throw std::runtime_error("call to vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed");

          fmt::print("capabilities.currentExtent: {}, {}\n", capabilities.currentExtent.width, capabilities.currentExtent.height);
          image_resource2D::image_conf iconf {
               .format            = VK_FORMAT_D32_SFLOAT,
               .extent            = capabilities.currentExtent,
               .mip_levels        = 1,
               .msaa              = VK_SAMPLE_COUNT_1_BIT,
               .tiling            = VK_IMAGE_TILING_OPTIMAL,
               .usage             = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
               .memory_properties = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT
          };
          image_resource2D::view_conf vconf { .aspect = VK_IMAGE_ASPECT_DEPTH_BIT };
          return swapchain(window, gpu, vulkan_core, capabilities, iconf, vconf);
     }

     ~swapchain() {
          for (auto& image_view : swapchain_image_views_)
               vkDestroyImageView(device_->logical(), image_view, vulkan_core_->allocator());
          swapchain_image_views_.clear();

          vkDestroySwapchainKHR(device_->logical(), swapchain_, vulkan_core_->allocator());
     }

     std::optional<uint32_t> try_next_image_index(VkSemaphore semaphore, VkFence fence) {
          uint32_t index {};
          if (vkAcquireNextImageKHR(device_->logical(), swapchain_, 4000000000, semaphore, fence, &index) == VK_SUCCESS)
               return index;
          return std::optional<uint32_t>();
     }

     void present(const uint32_t& index, const std::vector<VkSemaphore>& semaphores) {
          VkPresentInfoKHR present_info {
               .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
               .pNext              = nullptr,
               .waitSemaphoreCount = static_cast<uint32_t>(semaphores.size()),
               .pWaitSemaphores    = semaphores.data(),
               .swapchainCount     = 1,
               .pSwapchains        = &swapchain_,
               .pImageIndices      = &index,
               .pResults           = nullptr
          };
          if (vkQueuePresentKHR(device_->present(), &present_info) == VK_SUCCESS)
               return;
     }
     
     auto format() { return swapchain_format_; }
     auto depth_format() { return depth_resource_.format(); }

  private:
     swapchain(surface* window, device* gpu, core* vulkan_core, const VkSurfaceCapabilitiesKHR& capabilities, image_resource2D::image_conf iconf, image_resource2D::view_conf vconf)
        : device_(gpu)
        , vulkan_core_(vulkan_core)
        , swapchain_extent_(capabilities.currentExtent)
        , swapchain_format_(VK_FORMAT_B8G8R8A8_SRGB)
        , depth_resource_(device_, vulkan_core_, iconf, vconf) {
          create_swapchain(window, capabilities);
          initialize_swapchain_images();
          create_image_views();
     }

     void create_swapchain(surface* window, const VkSurfaceCapabilitiesKHR& capabilities) {
          uint32_t                 combo_queue_index { 0 };
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
               .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
               .queueFamilyIndexCount = 1,
               .pQueueFamilyIndices   = &combo_queue_index,
               .preTransform          = capabilities.currentTransform,
               .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
               .presentMode           = VK_PRESENT_MODE_MAILBOX_KHR,
               .clipped               = VK_TRUE,
               .oldSwapchain          = nullptr
          };

          if (vkCreateSwapchainKHR(device_->logical(), &swapchain_create_info, vulkan_core_->allocator(), &swapchain_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateSwapchainKHR failed");
     }

     void initialize_swapchain_images() {
          uint32_t count {};
          if (vkGetSwapchainImagesKHR(device_->logical(), swapchain_, &count, nullptr) != VK_SUCCESS)
               throw std::runtime_error("call to vkGetSwapchainImagesKHR failed");

          swapchain_images_.resize(count);
          if (vkGetSwapchainImagesKHR(device_->logical(), swapchain_, &count, swapchain_images_.data()) != VK_SUCCESS)
               throw std::runtime_error("call to vkGetSwapchainImagesKHR failed");
     }

     VkImageView create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels = 1) {
          VkImageSubresourceRange subresource {
               .aspectMask     = aspect_flags,
               .baseMipLevel   = 0,
               .levelCount     = mip_levels,
               .baseArrayLayer = 0,
               .layerCount     = 1
          };
          VkImageViewCreateInfo image_view_create_info {
               .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
               .pNext            = nullptr,
               .flags            = {},
               .image            = image,
               .viewType         = VK_IMAGE_VIEW_TYPE_2D,
               .format           = format,
               .components       = {},
               .subresourceRange = subresource
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

     device*                  device_;
     core*                    vulkan_core_;
     VkSwapchainKHR           swapchain_;
     VkExtent2D               swapchain_extent_;
     std::vector<VkImage>     swapchain_images_;
     VkFormat                 swapchain_format_;
     std::vector<VkImageView> swapchain_image_views_;
     image_resource2D         depth_resource_;
};
