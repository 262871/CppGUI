#pragma once

#include "Core.hpp"
#include "Device.hpp"
#include "ImageResource2D.hpp"
#include "Surface.hpp"

#include <optional>
#include <vector>

class Swapchain {
  public:
     static Swapchain make(Surface* surface, Device* device, Core* core) {
          VkSurfaceCapabilitiesKHR capabilities;
          if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physical(), surface->surfaceKHR(), &capabilities) != VK_SUCCESS)
               throw std::runtime_error("call to vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed");

          fmt::print("capabilities.currentExtent: {}, {}\n", capabilities.currentExtent.width, capabilities.currentExtent.height);
          ImageResource2D::ImageConf iConf {
               .format            = VK_FORMAT_D32_SFLOAT,
               .extent            = capabilities.currentExtent,
               .mipLevels        = 1,
               .msaa              = VK_SAMPLE_COUNT_1_BIT,
               .tiling            = VK_IMAGE_TILING_OPTIMAL,
               .usage             = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
               .memoryProperties = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT
          };
          ImageResource2D::ViewConf vConf { .aspect = VK_IMAGE_ASPECT_DEPTH_BIT };
          return Swapchain(surface, device, core, capabilities, iConf, vConf);
     }

     ~Swapchain() {
          for (auto& image_view : swapchainImageViews_)
               vkDestroyImageView(device_->logical(), image_view, core_->allocator());
          swapchainImageViews_.clear();

          vkDestroySwapchainKHR(device_->logical(), swapchain_, core_->allocator());
     }

     std::optional<uint32_t> tryNextImageIndex(VkSemaphore semaphore, VkFence fence) {
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
     auto extent() { return swapchainExtent_; }
     auto format() { return swapchainFormat_; }
     auto depthFormat() { return depthResource_.format(); }
     auto imageCount() { return swapchainImages_.size(); }

  private:
     Swapchain(Surface* surface, Device* device, Core* core, const VkSurfaceCapabilitiesKHR& capabilities, ImageResource2D::ImageConf iConf, ImageResource2D::ViewConf vConf)
        : device_(device)
        , core_(core)
        , swapchainExtent_(capabilities.currentExtent)
        , swapchainFormat_(VK_FORMAT_B8G8R8A8_SRGB)
        , depthResource_(device_, core_, iConf, vConf) {
          create_swapchain(surface, capabilities);
          initialize_swapchain_images();
          create_image_views();
     }

     void create_swapchain(Surface* surface, const VkSurfaceCapabilitiesKHR& capabilities) {
          uint32_t                 combo_queue_index { 0 };
          VkSwapchainCreateInfoKHR swapchain_create_info {
               .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
               .pNext                 = nullptr,
               .flags                 = {},
               .surface               = surface->surfaceKHR(),
               .minImageCount         = capabilities.minImageCount > 3 ? capabilities.minImageCount : capabilities.maxImageCount > 3 ? 3 : capabilities.maxImageCount,
               .imageFormat           = swapchainFormat_,
               .imageColorSpace       = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
               .imageExtent           = swapchainExtent_,
               .imageArrayLayers      = 1,
               .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
               .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
               .queueFamilyIndexCount = 1,
               .pQueueFamilyIndices   = &combo_queue_index,
               .preTransform          = capabilities.currentTransform,
               .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
               .presentMode           = VK_PRESENT_MODE_FIFO_KHR,
               .clipped               = VK_TRUE,
               .oldSwapchain          = nullptr
          };

          if (vkCreateSwapchainKHR(device_->logical(), &swapchain_create_info, core_->allocator(), &swapchain_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateSwapchainKHR failed");
     }

     void initialize_swapchain_images() {
          uint32_t count {};
          if (vkGetSwapchainImagesKHR(device_->logical(), swapchain_, &count, nullptr) != VK_SUCCESS)
               throw std::runtime_error("call to vkGetSwapchainImagesKHR failed");

          swapchainImages_.resize(count);
          if (vkGetSwapchainImagesKHR(device_->logical(), swapchain_, &count, swapchainImages_.data()) != VK_SUCCESS)
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
          if (vkCreateImageView(device_->logical(), &image_view_create_info, core_->allocator(), &image_view) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateImageView failed");
          return image_view;
     }

     void create_image_views() {
          swapchainImageViews_.reserve(swapchainImages_.size());
          for (size_t i = 0; i != swapchainImages_.size(); ++i)
               swapchainImageViews_.push_back(create_image_view(swapchainImages_[i], swapchainFormat_, VK_IMAGE_ASPECT_COLOR_BIT));
     }

     Device*                  device_;
     Core*                    core_;
     VkSwapchainKHR           swapchain_;
     VkExtent2D               swapchainExtent_;
     std::vector<VkImage>     swapchainImages_;
     VkFormat                 swapchainFormat_;
     std::vector<VkImageView> swapchainImageViews_;
     ImageResource2D          depthResource_;
};
