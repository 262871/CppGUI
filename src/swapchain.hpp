#pragma once

#include "core.hpp"
#include "device.hpp"
#include "surface.hpp"

#include <optional>
#include <vector>

class image_resource2D {
  public:
     struct image_conf {
          VkFormat              format;
          VkExtent2D            extent;
          uint32_t              mip_levels { 1 };
          VkSampleCountFlagBits msaa { VK_SAMPLE_COUNT_1_BIT };
          VkImageTiling         tiling { VK_IMAGE_TILING_OPTIMAL };
          VkImageUsageFlags     usage;
          VkMemoryPropertyFlags memory_properties;
     };
     struct view_conf {
          VkImageAspectFlags aspect;
     };
     image_resource2D() = default;
     image_resource2D(device* gpu, core* vulkan_core, const image_conf& iconf, const view_conf& vconf)
        : device_(gpu)
        , vulkan_core_(vulkan_core)
        , extent_(iconf.extent)
        , format_(iconf.format) {
          create_image(iconf);
          create_memory(iconf.memory_properties);
          create_image_view(vconf.aspect);
     }

     ~image_resource2D() {
          if (device_ != nullptr) {
               vkDestroyImageView(device_->logical(), image_view_, vulkan_core_->allocator());
               vkDestroyImage(device_->logical(), image_, vulkan_core_->allocator());
               vkFreeMemory(device_->logical(), memory_, vulkan_core_->allocator());
          }
     }

  private:
     void create_image(image_conf iconf) {
          VkImageCreateInfo image_create_info {
               .sType     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
               .pNext     = nullptr,
               .flags     = {},
               .imageType = VK_IMAGE_TYPE_2D,
               .format    = format_,
               .extent    = {
                     .width  = extent_.width,
                     .height = extent_.height,
                     .depth  = 1 },
               .mipLevels             = iconf.mip_levels,
               .arrayLayers           = 1,
               .samples               = iconf.msaa,
               .tiling                = iconf.tiling,
               .usage                 = iconf.usage,
               .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
               .queueFamilyIndexCount = 0,
               .pQueueFamilyIndices   = nullptr,
               .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED
          };
          if (vkCreateImage(device_->logical(), &image_create_info, vulkan_core_->allocator(), &image_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateImage failed");
     }

     uint32_t find_memory_index(uint32_t type_bits, VkMemoryPropertyFlags memory_property_flags) {
          VkPhysicalDeviceMemoryProperties physical_mem_props {};
          vkGetPhysicalDeviceMemoryProperties(device_->physical(), &physical_mem_props);

          for (uint32_t i { 0 }; i != physical_mem_props.memoryTypeCount; ++i)
               if (type_bits & (0b1 << i) && (physical_mem_props.memoryTypes[i].propertyFlags & memory_property_flags) == memory_property_flags)
                    return i;

          throw std::runtime_error("call to findMemory type failed to find memory");
     }
     void create_memory(VkMemoryPropertyFlags memory_properties) {
          VkMemoryRequirements memory_requirements;
          vkGetImageMemoryRequirements(device_->logical(), image_, &memory_requirements);

          VkMemoryAllocateInfo memory_allocate_info {
               .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
               .pNext           = nullptr,
               .allocationSize  = memory_requirements.size,
               .memoryTypeIndex = find_memory_index(memory_requirements.memoryTypeBits, memory_properties)
          };

          if (vkAllocateMemory(device_->logical(), &memory_allocate_info, vulkan_core_->allocator(), &memory_) != VK_SUCCESS)
               throw std::runtime_error("call to vkAllocateMemory failed");

          if (vkBindImageMemory(device_->logical(), image_, memory_, 0) != VK_SUCCESS)
               throw std::runtime_error("call to vkBindImageMemory failed");
     }
     void create_image_view(VkImageAspectFlags aspect_flags, uint32_t mip_levels = 1) {
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
               .image            = image_,
               .viewType         = VK_IMAGE_VIEW_TYPE_2D,
               .format           = format_,
               .components       = {},
               .subresourceRange = subresource
          };
          if (vkCreateImageView(device_->logical(), &image_view_create_info, vulkan_core_->allocator(), &image_view_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateImageView failed");
     }

     device*        device_ { nullptr };
     core*          vulkan_core_ { nullptr };
     VkExtent2D     extent_ { 0, 0 };
     VkFormat       format_ {};
     VkImage        image_ { nullptr };
     VkDeviceMemory memory_ { nullptr };
     VkImageView    image_view_ { nullptr };
};

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

          uint32_t combo_queue_index { 0 };

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

          uint32_t count {};
          if (vkGetSwapchainImagesKHR(device_->logical(), swapchain_, &count, nullptr) != VK_SUCCESS)
               throw std::runtime_error("call to vkGetSwapchainImagesKHR failed");

          swapchain_images_.resize(count);
          if (vkGetSwapchainImagesKHR(device_->logical(), swapchain_, &count, swapchain_images_.data()) != VK_SUCCESS)
               throw std::runtime_error("call to vkGetSwapchainImagesKHR failed");

          create_image_views();
          image_resource2D::image_conf iconf {
               .format            = VK_FORMAT_D32_SFLOAT,
               .extent            = swapchain_extent_,
               .mip_levels        = 1,
               .msaa              = VK_SAMPLE_COUNT_1_BIT,
               .tiling            = VK_IMAGE_TILING_OPTIMAL,
               .usage             = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
               .memory_properties = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT
          };
          image_resource2D::view_conf vconf { .aspect = VK_IMAGE_ASPECT_DEPTH_BIT };
          depth_resource_ = std::make_unique<image_resource2D>(device_, vulkan_core_, iconf, vconf);
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

  private:
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

     device*                           device_;
     core*                             vulkan_core_;
     VkSwapchainKHR                    swapchain_;
     VkExtent2D                        swapchain_extent_;
     std::vector<VkImage>              swapchain_images_;
     VkFormat                          swapchain_format_;
     std::vector<VkImageView>          swapchain_image_views_;
     std::unique_ptr<image_resource2D> depth_resource_;
};
