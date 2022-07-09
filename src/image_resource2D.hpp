#pragma once

#include "core.hpp"
#include "device.hpp"

#include <volk.h>

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
     
     auto format() { return format_; }

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


