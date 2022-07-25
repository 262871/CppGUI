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
     ImageResource2D(Core* core, Device* device, const ImageConf& iConf, const ViewConf& vConf)
        : core_(core)
        , device_(device)
        , iConf_(iConf)
        , vConf_(vConf) {
          createImage();
          createMemory();
          createImageView();
     }
     
     void resize(const ImageConf& iConf, const ViewConf& vConf) {
          if (device_ != nullptr) {
               vkDestroyImageView(device_->logical(), imageView_, core_->allocator());
               vkDestroyImage(device_->logical(), image_, core_->allocator());
               vkFreeMemory(device_->logical(), memory_, core_->allocator());
          }
          iConf_ = iConf;
          vConf_ = vConf;
          createImage();
          createMemory();
          createImageView();
     }

     ~ImageResource2D() {
          if (device_ != nullptr) {
               vkDestroyImageView(device_->logical(), imageView_, core_->allocator());
               vkDestroyImage(device_->logical(), image_, core_->allocator());
               vkFreeMemory(device_->logical(), memory_, core_->allocator());
          }
     }

     auto view() { return imageView_; }
     auto format() { return iConf_.format; }
     auto msaa() { return iConf_.msaa; }

  private:
     void createImage() {
          VkImageCreateInfo image_create_info {
               .sType     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
               .pNext     = nullptr,
               .flags     = {},
               .imageType = VK_IMAGE_TYPE_2D,
               .format    = iConf_.format,
               .extent    = {
                     .width  = iConf_.extent.width,
                     .height = iConf_.extent.height,
                     .depth  = 1 },
               .mipLevels             = iConf_.mipLevels,
               .arrayLayers           = 1,
               .samples               = iConf_.msaa,
               .tiling                = iConf_.tiling,
               .usage                 = iConf_.usage,
               .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
               .queueFamilyIndexCount = 0,
               .pQueueFamilyIndices   = nullptr,
               .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED
          };
          if (vkCreateImage(device_->logical(), &image_create_info, core_->allocator(), &image_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateImage failed");
     }

     uint32_t findMemoryIndex(uint32_t typeBits, VkMemoryPropertyFlags memoryPropertyFlags) {
          VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties {};
          vkGetPhysicalDeviceMemoryProperties(device_->physical(), &physicalDeviceMemoryProperties);

          for (uint32_t i { 0 }; i != physicalDeviceMemoryProperties.memoryTypeCount; ++i)
               if (typeBits & (0b1 << i) && (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
                    return i;

          throw std::runtime_error("call to findMemory type failed to find memory");
     }
     void createMemory() {
          VkMemoryRequirements memoryRequirements;
          vkGetImageMemoryRequirements(device_->logical(), image_, &memoryRequirements);

          VkMemoryAllocateInfo memoryAllocateInfo {
               .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
               .pNext           = nullptr,
               .allocationSize  = memoryRequirements.size,
               .memoryTypeIndex = findMemoryIndex(memoryRequirements.memoryTypeBits, iConf_.memoryProperties)
          };

          if (vkAllocateMemory(device_->logical(), &memoryAllocateInfo, core_->allocator(), &memory_) != VK_SUCCESS)
               throw std::runtime_error("call to vkAllocateMemory failed");

          if (vkBindImageMemory(device_->logical(), image_, memory_, 0) != VK_SUCCESS)
               throw std::runtime_error("call to vkBindImageMemory failed");
     }
     void createImageView(uint32_t mipLevels = 1) {
          VkImageSubresourceRange subresource {
               .aspectMask     = vConf_.aspect,
               .baseMipLevel   = 0,
               .levelCount     = mipLevels,
               .baseArrayLayer = 0,
               .layerCount     = 1
          };
          VkImageViewCreateInfo imageViewCreateInfo {
               .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
               .pNext            = nullptr,
               .flags            = {},
               .image            = image_,
               .viewType         = VK_IMAGE_VIEW_TYPE_2D,
               .format           = iConf_.format,
               .components       = {},
               .subresourceRange = subresource
          };
          if (vkCreateImageView(device_->logical(), &imageViewCreateInfo, core_->allocator(), &imageView_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateImageView failed");
     }

     Core*          core_;
     Device*        device_;
     ImageConf      iConf_;
     ViewConf       vConf_;
     VkImage        image_;
     VkDeviceMemory memory_;
     VkImageView    imageView_;
};
