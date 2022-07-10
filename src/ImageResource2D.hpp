#pragma once

#include "Core.hpp"
#include "Device.hpp"

class ImageResource2D {
  public:
     struct ImageConf {
          VkFormat              format;
          VkExtent2D            extent;
          uint32_t              mipLevels { 1 };
          VkSampleCountFlagBits msaa { VK_SAMPLE_COUNT_1_BIT };
          VkImageTiling         tiling { VK_IMAGE_TILING_OPTIMAL };
          VkImageUsageFlags     usage;
          VkMemoryPropertyFlags memoryProperties;
     };
     struct ViewConf {
          VkImageAspectFlags aspect;
     };
     ImageResource2D(Device* device, Core* core, const ImageConf& iConf, const ViewConf& vConf)
        : device_(device)
        , core_(core)
        , extent_(iConf.extent)
        , format_(iConf.format) {
          createImage(iConf);
          createMemory(iConf.memoryProperties);
          createImageView(vConf.aspect);
     }

     ~ImageResource2D() {
          if (device_ != nullptr) {
               vkDestroyImageView(device_->logical(), imageView_, core_->allocator());
               vkDestroyImage(device_->logical(), image_, core_->allocator());
               vkFreeMemory(device_->logical(), memory_, core_->allocator());
          }
     }

     auto format() { return format_; }

  private:
     void createImage(ImageConf iConf) {
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
               .mipLevels             = iConf.mipLevels,
               .arrayLayers           = 1,
               .samples               = iConf.msaa,
               .tiling                = iConf.tiling,
               .usage                 = iConf.usage,
               .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
               .queueFamilyIndexCount = 0,
               .pQueueFamilyIndices   = nullptr,
               .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED
          };
          if (vkCreateImage(device_->logical(), &image_create_info, core_->allocator(), &image_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateImage failed");
     }

     uint32_t findMemoryIndex(uint32_t type_bits, VkMemoryPropertyFlags memory_property_flags) {
          VkPhysicalDeviceMemoryProperties physical_mem_props {};
          vkGetPhysicalDeviceMemoryProperties(device_->physical(), &physical_mem_props);

          for (uint32_t i { 0 }; i != physical_mem_props.memoryTypeCount; ++i)
               if (type_bits & (0b1 << i) && (physical_mem_props.memoryTypes[i].propertyFlags & memory_property_flags) == memory_property_flags)
                    return i;

          throw std::runtime_error("call to findMemory type failed to find memory");
     }
     void createMemory(VkMemoryPropertyFlags memoryProperties) {
          VkMemoryRequirements memory_requirements;
          vkGetImageMemoryRequirements(device_->logical(), image_, &memory_requirements);

          VkMemoryAllocateInfo memory_allocate_info {
               .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
               .pNext           = nullptr,
               .allocationSize  = memory_requirements.size,
               .memoryTypeIndex = findMemoryIndex(memory_requirements.memoryTypeBits, memoryProperties)
          };

          if (vkAllocateMemory(device_->logical(), &memory_allocate_info, core_->allocator(), &memory_) != VK_SUCCESS)
               throw std::runtime_error("call to vkAllocateMemory failed");

          if (vkBindImageMemory(device_->logical(), image_, memory_, 0) != VK_SUCCESS)
               throw std::runtime_error("call to vkBindImageMemory failed");
     }
     void createImageView(VkImageAspectFlags aspect_flags, uint32_t mipLevels = 1) {
          VkImageSubresourceRange subresource {
               .aspectMask     = aspect_flags,
               .baseMipLevel   = 0,
               .levelCount     = mipLevels,
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
          if (vkCreateImageView(device_->logical(), &image_view_create_info, core_->allocator(), &imageView_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateImageView failed");
     }

     Device*        device_ { nullptr };
     Core*          core_ { nullptr };
     VkExtent2D     extent_ { 0, 0 };
     VkFormat       format_ {};
     VkImage        image_ { nullptr };
     VkDeviceMemory memory_ { nullptr };
     VkImageView    imageView_ { nullptr };
};
