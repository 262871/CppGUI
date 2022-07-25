#pragma once

#include "Core.hpp"
#include "Device.hpp"
#include "Surface.hpp"

#include <optional>
#include <vector>

class Swapchain {
  public:
     std::optional<uint32_t> tryNextImageIndex(VkSemaphore semaphore, VkFence fence) {
          uint32_t index {};
          if (vkAcquireNextImageKHR(device_->logical(), swapchain_, 4000000000, semaphore, fence, &index) == VK_SUCCESS)
               return index;
          return std::optional<uint32_t>();
     }
     void present(const uint32_t& index, const std::vector<VkSemaphore>& semaphores) {
          VkPresentInfoKHR presentInfo {
               .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
               .pNext              = nullptr,
               .waitSemaphoreCount = static_cast<uint32_t>(semaphores.size()),
               .pWaitSemaphores    = semaphores.data(),
               .swapchainCount     = 1,
               .pSwapchains        = &swapchain_,
               .pImageIndices      = &index,
               .pResults           = nullptr
          };
          if (vkQueuePresentKHR(device_->present(), &presentInfo) == VK_SUCCESS)
               return;
     }
     auto extent() { return swapchainExtent_; }
     auto format() { return swapchainFormat_; }
     auto imageCount() { return imageCount_; }
     auto imageViews() { return swapchainImageViews_; }
     auto& get() { return swapchain_; }
     void resize() {
          for (auto& image_view : swapchainImageViews_)
               vkDestroyImageView(device_->logical(), image_view, core_->allocator());
          swapchainImageViews_.clear();
          vkDestroySwapchainKHR(device_->logical(), swapchain_, core_->allocator());

          VkSurfaceCapabilitiesKHR surfaceCapabilities;
          if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_->physical(), surface_->surfaceKHR(), &surfaceCapabilities) != VK_SUCCESS)
               throw std::runtime_error("call to vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed");

          swapchainExtent_ = surfaceCapabilities.currentExtent;

          createSwapchain(surfaceCapabilities);
          initializeSwapchainImages();
          createImageViews();
     }
     Swapchain(Core* core, Surface* surface, Device* device)
        : core_(core)
        , surface_(surface)
        , device_(device) {
          VkSurfaceCapabilitiesKHR surfaceCapabilities;
          if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_->physical(), surface_->surfaceKHR(), &surfaceCapabilities) != VK_SUCCESS)
               throw std::runtime_error("call to vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed");
          fmt::print("capabilities.currentExtent: {}, {}\n", surfaceCapabilities.currentExtent.width, surfaceCapabilities.currentExtent.height);
          
          swapchainExtent_ = surfaceCapabilities.currentExtent;
          createSwapchain(surfaceCapabilities);
          initializeSwapchainImages();
          createImageViews();
          fmt::print("Swapchain image count: {}\n", imageCount_);
     }
     ~Swapchain() {
          for (auto& image_view : swapchainImageViews_)
               vkDestroyImageView(device_->logical(), image_view, core_->allocator());
          swapchainImageViews_.clear();
          vkDestroySwapchainKHR(device_->logical(), swapchain_, core_->allocator());
     }

  private:
     void createSwapchain(const VkSurfaceCapabilitiesKHR& surfaceCapabilities) {
          uint32_t comboQueueIndex { 0 };
          
          uint32_t imageCount;
          if (surfaceCapabilities.minImageCount > 2)
               imageCount = surfaceCapabilities.minImageCount;
          else if (surfaceCapabilities.maxImageCount > 3)
               imageCount = 3;
          else
               imageCount = surfaceCapabilities.maxImageCount;

          VkSwapchainCreateInfoKHR swapchainCreateInfo {
               .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
               .pNext                 = nullptr,
               .flags                 = {},
               .surface               = surface_->surfaceKHR(),
               .minImageCount         = imageCount,
               .imageFormat           = swapchainFormat_,
               .imageColorSpace       = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
               .imageExtent           = swapchainExtent_,
               .imageArrayLayers      = 1,
               .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
               .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
               .queueFamilyIndexCount = 1,
               .pQueueFamilyIndices   = &comboQueueIndex,
               .preTransform          = surfaceCapabilities.currentTransform,
               .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
               .presentMode           = VK_PRESENT_MODE_FIFO_KHR,
               .clipped               = VK_TRUE,
               .oldSwapchain          = nullptr
          };

          if (vkCreateSwapchainKHR(device_->logical(), &swapchainCreateInfo, core_->allocator(), &swapchain_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateSwapchainKHR failed");
     }

     void initializeSwapchainImages() {
          if (vkGetSwapchainImagesKHR(device_->logical(), swapchain_, &imageCount_, nullptr) != VK_SUCCESS)
               throw std::runtime_error("call to vkGetSwapchainImagesKHR failed");

          swapchainImages_.resize(imageCount_);
          if (vkGetSwapchainImagesKHR(device_->logical(), swapchain_, &imageCount_, swapchainImages_.data()) != VK_SUCCESS)
               throw std::runtime_error("call to vkGetSwapchainImagesKHR failed");
     }

     VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels = 1) {
          VkImageSubresourceRange imageSubresourceRange {
               .aspectMask     = aspectFlags,
               .baseMipLevel   = 0,
               .levelCount     = mipLevels,
               .baseArrayLayer = 0,
               .layerCount     = 1
          };
          VkImageViewCreateInfo imageViewCreateInfo {
               .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
               .pNext            = nullptr,
               .flags            = {},
               .image            = image,
               .viewType         = VK_IMAGE_VIEW_TYPE_2D,
               .format           = format,
               .components       = {},
               .subresourceRange = imageSubresourceRange
          };
          VkImageView imageView {};
          if (vkCreateImageView(device_->logical(), &imageViewCreateInfo, core_->allocator(), &imageView) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateImageView failed");
          return imageView;
     }

     void createImageViews() {
          swapchainImageViews_.reserve(imageCount_);
          for (size_t i = 0; i != imageCount_; ++i)
               swapchainImageViews_.push_back(createImageView(swapchainImages_[i], swapchainFormat_, VK_IMAGE_ASPECT_COLOR_BIT));
     }

     Core*    core_;
     Surface* surface_;
     Device*  device_;

     VkExtent2D               swapchainExtent_;
     VkFormat                 swapchainFormat_ {VK_FORMAT_B8G8R8A8_SRGB};
     VkSwapchainKHR           swapchain_;
     uint32_t                 imageCount_;
     std::vector<VkImage>     swapchainImages_;
     std::vector<VkImageView> swapchainImageViews_;
};
