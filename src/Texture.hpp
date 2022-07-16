#pragma once

#include "Buffer.hpp"
#include "CommandPool.hpp"
#include "Core.hpp"
#include "Device.hpp"

const auto TEXTURE_PATH = "textures\\viking_room.png";
class Texture {
     Core*          core_;
     Device*        device_;
     CommandPool*   commandPool_;
     VkExtent2D     extent_;
     uint32_t       mipLevels_;
     VkImage        textureImage_;
     VkImageView    textureImageView_;
     VkSampler      textureImageSampler_;
     VkDeviceMemory textureImageMemory_;

  public:
     Texture(Core* core, Device* device, CommandPool* commandPool)
        : core_(core)
        , device_(device)
        , commandPool_(commandPool) {
          int      textureWidth {};
          int      textureHeight {};
          int      textureChannels {};
          stbi_uc* pixels        = stbi_load(TEXTURE_PATH, &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);
          mipLevels_             = static_cast<uint32_t>(std::floor(std::log2(std::max(textureWidth, textureHeight)))) + 1;
          VkDeviceSize imageSize = textureWidth * textureHeight * 4;
          if (pixels == nullptr)
               throw std::runtime_error("failed to load texture image");

          extent_.width  = static_cast<uint32_t>(textureWidth);
          extent_.height = static_cast<uint32_t>(textureHeight);

          auto stagingBuffer = Buffer<stbi_uc>::makeStaging(core_, device_, commandPool_, imageSize);

          stagingBuffer.write(pixels);
          stbi_image_free(pixels);

          createImage();
          transitionImageLayout();
          copyBufferToImage(stagingBuffer.get());
          generateMipmaps();
          createImageView();
          createTextureSampler();
     }

     ~Texture() {
          vkDestroySampler(device_->logical(), textureImageSampler_, core_->allocator());
          vkDestroyImageView(device_->logical(), textureImageView_, core_->allocator());
          vkDestroyImage(device_->logical(), textureImage_, core_->allocator());
          vkFreeMemory(device_->logical(), textureImageMemory_, core_->allocator());
     }

     auto& sampler() { return textureImageSampler_; }
     auto& view() { return textureImageView_; }

  private:
     uint32_t findMemoryIndex(uint32_t typeBits, VkMemoryPropertyFlags memoryPropertyFlags) {
          VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
          vkGetPhysicalDeviceMemoryProperties(device_->physical(), &physicalDeviceMemoryProperties);

          for (uint32_t i { 0 }; i != physicalDeviceMemoryProperties.memoryTypeCount; ++i)
               if (typeBits & (0b1 << i) && (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
                    return i;

          throw std::runtime_error("call to findMemory type failed to find memory");
     }

     void createImage() {
          VkImageCreateInfo imageCreateInfo {
               .sType     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
               .pNext     = nullptr,
               .flags     = {},
               .imageType = VK_IMAGE_TYPE_2D,
               .format    = VK_FORMAT_R8G8B8A8_SRGB,
               .extent    = {
                     .width  = extent_.width,
                     .height = extent_.height,
                     .depth  = 1 },
               .mipLevels             = mipLevels_,
               .arrayLayers           = 1,
               .samples               = VK_SAMPLE_COUNT_1_BIT,
               .tiling                = VK_IMAGE_TILING_OPTIMAL,
               .usage                 = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
               .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
               .queueFamilyIndexCount = 0,
               .pQueueFamilyIndices   = nullptr,
               .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED
          };

          if (vkCreateImage(device_->logical(), &imageCreateInfo, core_->allocator(), &textureImage_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateImage failed");

          VkMemoryRequirements memoryRequirements;
          vkGetImageMemoryRequirements(device_->logical(), textureImage_, &memoryRequirements);

          VkMemoryAllocateInfo memoryAllocateInfo {
               .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
               .pNext           = nullptr,
               .allocationSize  = memoryRequirements.size,
               .memoryTypeIndex = findMemoryIndex(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
          };

          if (vkAllocateMemory(device_->logical(), &memoryAllocateInfo, core_->allocator(), &textureImageMemory_) != VK_SUCCESS)
               throw std::runtime_error("call to vkAllocateMemory failed");
          if (vkBindImageMemory(device_->logical(), textureImage_, textureImageMemory_, 0) != VK_SUCCESS)
               throw std::runtime_error("call to vkBindImageMemory failed");
     }

     void transitionImageLayout() {
          commandPool_->singleTimeCommand([this](auto commandBuffer) {
               VkImageMemoryBarrier imageMemoryBarrier {
                    .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext               = nullptr,
                    .srcAccessMask       = VK_ACCESS_NONE,
                    .dstAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,
                    .newLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image               = textureImage_,
                    .subresourceRange    = {
                          .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                          .baseMipLevel   = 0,
                          .levelCount     = mipLevels_,
                          .baseArrayLayer = 0,
                          .layerCount     = 1 },
               };

               vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
          },
             Device::QueuePriority::TRANSFER_MEDIUM);
     }

     void copyBufferToImage(VkBuffer buffer) {
          commandPool_->singleTimeCommand([this, buffer](auto commandBuffer) {
               VkBufferImageCopy region {
                    .bufferOffset      = 0,
                    .bufferRowLength   = 0,
                    .bufferImageHeight = 0,
                    .imageSubresource  = {
                        .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                        .mipLevel       = 0,
                        .baseArrayLayer = 0,
                        .layerCount     = 1 },
                    .imageOffset = { .x = 0, .y = 0, .z = 0 },
                    .imageExtent = { .width = extent_.width, .height = extent_.height, .depth = 1 },
               };
               vkCmdCopyBufferToImage(commandBuffer, buffer, textureImage_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
          },
             Device::QueuePriority::TRANSFER_MEDIUM);
     }

     void generateMipmaps() {
          VkFormatProperties properties {};
          vkGetPhysicalDeviceFormatProperties(device_->physical(), VK_FORMAT_R8G8B8A8_SRGB, &properties);
          if (!(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
               throw std::runtime_error("texture image format does not support linear blitting in generateMipmaps");

          commandPool_->singleTimeCommand([this, &properties](auto commandBuffer) {
               VkImageMemoryBarrier barrier {
                    .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext               = nullptr,
                    .srcAccessMask       = {},
                    .dstAccessMask       = {},
                    .oldLayout           = {},
                    .newLayout           = {},
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image               = textureImage_,
                    .subresourceRange    = VkImageSubresourceRange {
                          .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                          .baseMipLevel   = 0,
                          .levelCount     = 1,
                          .baseArrayLayer = 0,
                          .layerCount     = 1 }
               };

               int32_t width  = extent_.width;
               int32_t height = extent_.height;

               for (uint32_t i = 1; i != mipLevels_; ++i) {
                    barrier.subresourceRange.baseMipLevel = i - 1;
                    barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                    barrier.newLayout                     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
                    barrier.dstAccessMask                 = VK_ACCESS_TRANSFER_READ_BIT;
                    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
                    VkImageBlit blit {
                         .srcSubresource = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = i - 1, .baseArrayLayer = 0, .layerCount = 1 },
                         .srcOffsets     = { { 0, 0, 0 }, { width, height, 1 } },
                         .dstSubresource = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = i, .baseArrayLayer = 0, .layerCount = 1 },
                         .dstOffsets     = { { 0, 0, 0 }, { width > 1 ? width / 2 : 1, height > 1 ? height / 2 : 1, 1 } },
                    };
                    vkCmdBlitImage(commandBuffer, textureImage_, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, textureImage_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);
                    barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
                    if (width > 1)
                         width /= 2;
                    if (height > 1)
                         height /= 2;
               }
               barrier.subresourceRange.baseMipLevel = mipLevels_ - 1;
               barrier.oldLayout                     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
               barrier.newLayout                     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
               barrier.srcAccessMask                 = VK_ACCESS_TRANSFER_WRITE_BIT;
               barrier.dstAccessMask                 = VK_ACCESS_SHADER_READ_BIT;
               vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
          },
             Device::QueuePriority::GRAPHICS_MEDIUM);
     }

     void createImageView() {
          VkImageViewCreateInfo imageViewCreateInfo {
               .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
               .pNext      = nullptr,
               .flags      = {},
               .image      = textureImage_,
               .viewType   = VK_IMAGE_VIEW_TYPE_2D,
               .format     = VK_FORMAT_R8G8B8A8_SRGB,
               .components = {
                  .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                  .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                  .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                  .a = VK_COMPONENT_SWIZZLE_IDENTITY },
               .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = mipLevels_, .baseArrayLayer = 0, .layerCount = 1 }
          };
          if (vkCreateImageView(device_->logical(), &imageViewCreateInfo, core_->allocator(), &textureImageView_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateImageView failed");
     }

     void createTextureSampler() {
          VkPhysicalDeviceProperties physicalDeviceProperties {};
          vkGetPhysicalDeviceProperties(device_->physical(), &physicalDeviceProperties);

          VkSamplerCreateInfo samplerCreateInfo {
               .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
               .pNext                   = nullptr,
               .flags                   = {},
               .magFilter               = VK_FILTER_LINEAR,
               .minFilter               = VK_FILTER_LINEAR,
               .mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR,
               .addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
               .addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
               .addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
               .mipLodBias              = 0.f,
               .anisotropyEnable        = VK_TRUE,
               .maxAnisotropy           = physicalDeviceProperties.limits.maxSamplerAnisotropy,
               .compareEnable           = VK_FALSE,
               .compareOp               = VK_COMPARE_OP_ALWAYS,
               .minLod                  = 0.f,
               .maxLod                  = 1000.f,
               .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
               .unnormalizedCoordinates = VK_FALSE
          };

          if (vkCreateSampler(device_->logical(), &samplerCreateInfo, core_->allocator(), &textureImageSampler_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateSampler failed");
     }
};
