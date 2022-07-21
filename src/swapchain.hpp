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
          VkSurfaceCapabilitiesKHR surfaceCapabilities;
          if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physical(), surface->surfaceKHR(), &surfaceCapabilities) != VK_SUCCESS)
               throw std::runtime_error("call to vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed");
               
          fmt::print("capabilities.currentExtent: {}, {}\n", surfaceCapabilities.currentExtent.width, surfaceCapabilities.currentExtent.height);
               
          // -------------------------------------------------------------------------------------------------------------------//
          ImageResource2D::ImageConf iColorConf {
               .format           = VK_FORMAT_B8G8R8A8_SRGB,
               .extent           = surfaceCapabilities.currentExtent,
               .mipLevels        = 1,
               .msaa             = VK_SAMPLE_COUNT_1_BIT,
               .tiling           = VK_IMAGE_TILING_OPTIMAL,
               .usage            = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
               .memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
          };
          ImageResource2D::ViewConf vColorConf { .aspect = VK_IMAGE_ASPECT_COLOR_BIT };
          // -------------------------------------------------------------------------------------------------------------------//

          // -------------------------------------------------------------------------------------------------------------------//
          ImageResource2D::ImageConf iDepthConf {
               .format           = VK_FORMAT_D32_SFLOAT,
               .extent           = surfaceCapabilities.currentExtent,
               .mipLevels        = 1,
               .msaa             = VK_SAMPLE_COUNT_1_BIT,
               .tiling           = VK_IMAGE_TILING_OPTIMAL,
               .usage            = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
               .memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
          };
          ImageResource2D::ViewConf vDepthConf { .aspect = VK_IMAGE_ASPECT_DEPTH_BIT };
          // -------------------------------------------------------------------------------------------------------------------//

          return Swapchain(surface, device, core, surfaceCapabilities, iColorConf, vColorConf, iDepthConf, vDepthConf);
     }
     void update(Surface* surface) {
          for (auto& image_view : swapchainImageViews_)
               vkDestroyImageView(device_->logical(), image_view, core_->allocator());
          swapchainImageViews_.clear();
          vkDestroySwapchainKHR(device_->logical(), swapchain_, core_->allocator());
          
          VkSurfaceCapabilitiesKHR surfaceCapabilities;
          if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_->physical(), surface->surfaceKHR(), &surfaceCapabilities) != VK_SUCCESS)
               throw std::runtime_error("call to vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed");
          
          swapchainExtent_ = surfaceCapabilities.currentExtent;
          
          // -------------------------------------------------------------------------------------------------------------------//
          ImageResource2D::ImageConf iColorConf {
               .format           = VK_FORMAT_B8G8R8A8_SRGB,
               .extent           = surfaceCapabilities.currentExtent,
               .mipLevels        = 1,
               .msaa             = VK_SAMPLE_COUNT_1_BIT,
               .tiling           = VK_IMAGE_TILING_OPTIMAL,
               .usage            = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
               .memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
          };
          ImageResource2D::ViewConf vColorConf { .aspect = VK_IMAGE_ASPECT_COLOR_BIT };
          // -------------------------------------------------------------------------------------------------------------------//

          // -------------------------------------------------------------------------------------------------------------------//
          ImageResource2D::ImageConf iDepthConf {
               .format           = VK_FORMAT_D32_SFLOAT,
               .extent           = surfaceCapabilities.currentExtent,
               .mipLevels        = 1,
               .msaa             = VK_SAMPLE_COUNT_1_BIT,
               .tiling           = VK_IMAGE_TILING_OPTIMAL,
               .usage            = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
               .memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
          };
          ImageResource2D::ViewConf vDepthConf { .aspect = VK_IMAGE_ASPECT_DEPTH_BIT };
          // -------------------------------------------------------------------------------------------------------------------//
          
          colorResource_.resize(iColorConf, vColorConf);
          depthResource_.resize(iDepthConf, vDepthConf);
          createSwapchain(surface, surfaceCapabilities);
          initializeSwapchainImages();
          createImageViews();
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
     auto imageCount() { return imageCount_; }

     auto colorView() { return colorResource_.view(); }
     auto depthView() { return depthResource_.view(); }
     auto imageViews() { return swapchainImageViews_; }
     
     auto& get() { return swapchain_; }

  private:
     Swapchain(Surface* surface, Device* device, Core* core, const VkSurfaceCapabilitiesKHR& caps, ImageResource2D::ImageConf iColorConf, ImageResource2D::ViewConf vColorConf, ImageResource2D::ImageConf iDepthConf, ImageResource2D::ViewConf vDepthConf)
        : device_(device)
        , core_(core)
        , swapchainExtent_(caps.currentExtent)
        , swapchainFormat_(VK_FORMAT_B8G8R8A8_SRGB)
        , colorResource_(device_, core_, iColorConf, vColorConf)
        , depthResource_(device_, core_, iDepthConf, vDepthConf) {
          createSwapchain(surface, caps);
          initializeSwapchainImages();
          createImageViews();
          fmt::print("Swapchain image count: {}\n", imageCount_);
     }

     void createSwapchain(Surface* surface, const VkSurfaceCapabilitiesKHR& caps) {
          uint32_t combo_queue_index { 0 };
          uint32_t imageCount = 0;
          if (caps.minImageCount > 2)
               imageCount = caps.minImageCount;
          else if (caps.maxImageCount > 3)
               imageCount = 3;
          else
               imageCount = caps.maxImageCount;

          VkSwapchainCreateInfoKHR swapchain_create_info {
               .sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
               .pNext                 = nullptr,
               .flags                 = {},
               .surface               = surface->surfaceKHR(),
               .minImageCount         = imageCount,
               .imageFormat           = swapchainFormat_,
               .imageColorSpace       = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
               .imageExtent           = swapchainExtent_,
               .imageArrayLayers      = 1,
               .imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
               .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
               .queueFamilyIndexCount = 1,
               .pQueueFamilyIndices   = &combo_queue_index,
               .preTransform          = caps.currentTransform,
               .compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
               .presentMode           = VK_PRESENT_MODE_FIFO_KHR,
               .clipped               = VK_TRUE,
               .oldSwapchain          = nullptr
          };

          if (vkCreateSwapchainKHR(device_->logical(), &swapchain_create_info, core_->allocator(), &swapchain_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateSwapchainKHR failed");
     }

     void initializeSwapchainImages() {
          if (vkGetSwapchainImagesKHR(device_->logical(), swapchain_, &imageCount_, nullptr) != VK_SUCCESS)
               throw std::runtime_error("call to vkGetSwapchainImagesKHR failed");

          swapchainImages_.resize(imageCount_);
          if (vkGetSwapchainImagesKHR(device_->logical(), swapchain_, &imageCount_, swapchainImages_.data()) != VK_SUCCESS)
               throw std::runtime_error("call to vkGetSwapchainImagesKHR failed");
     }

     VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels = 1) {
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

     void createImageViews() {
          swapchainImageViews_.reserve(imageCount_);
          for (size_t i = 0; i != imageCount_; ++i)
               swapchainImageViews_.push_back(createImageView(swapchainImages_[i], swapchainFormat_, VK_IMAGE_ASPECT_COLOR_BIT));
     }

     Device*                  device_;
     Core*                    core_;
     VkExtent2D               swapchainExtent_;
     VkFormat                 swapchainFormat_;
     ImageResource2D          colorResource_;
     ImageResource2D          depthResource_;
     VkSwapchainKHR           swapchain_;
     uint32_t                 imageCount_;
     std::vector<VkImage>     swapchainImages_;
     std::vector<VkImageView> swapchainImageViews_;
};
