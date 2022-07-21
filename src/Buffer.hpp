#pragma once

#include "CommandPool.hpp"
#include "Core.hpp"
#include "Vertex.hpp"
#include "Device.hpp"

#include <glm/glm.hpp>

struct UniformBufferObject {
     glm::mat4 model;
     glm::mat4 view;
     glm::mat4 proj;
};

template <typename T>
class Buffer {
     Core*          core_;
     Device*        device_;
     CommandPool*   commandPool_;
     size_t         size_;
     VkBuffer       buffer_;
     VkDeviceMemory bufferMemory_;

     static uint32_t findMemoryType(Device* device, uint32_t typeFilter, VkMemoryPropertyFlags memoryPropertyFlags) {
          VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties {};
          vkGetPhysicalDeviceMemoryProperties(device->physical(), &vkPhysicalDeviceMemoryProperties);

          for (uint32_t i = 0; i != vkPhysicalDeviceMemoryProperties.memoryTypeCount; ++i)
               if (typeFilter & (0b1 << i) && (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
                    return i;

          throw std::runtime_error("call to findMemoryType failed");
     }

     static auto createBuffer(Core* core, Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
          VkBuffer           buffer;
          VkDeviceMemory     bufferMemory;
          VkBufferCreateInfo vkBufferCreateInfo {
               .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
               .pNext                 = nullptr,
               .flags                 = {},
               .size                  = size,
               .usage                 = usage,
               .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
               .queueFamilyIndexCount = 0,
               .pQueueFamilyIndices   = nullptr
          };
          if (vkCreateBuffer(device->logical(), &vkBufferCreateInfo, core->allocator(), &buffer) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateBuffer failed to create buffer");

          VkMemoryRequirements vkMemoryRequirements {};
          vkGetBufferMemoryRequirements(device->logical(), buffer, &vkMemoryRequirements);
          VkMemoryAllocateInfo vkMemoryAllocateInfo {
               .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
               .pNext           = nullptr,
               .allocationSize  = vkMemoryRequirements.size,
               .memoryTypeIndex = findMemoryType(device, vkMemoryRequirements.memoryTypeBits, properties)
          };
          if (vkAllocateMemory(device->logical(), &vkMemoryAllocateInfo, core->allocator(), &bufferMemory) != VK_SUCCESS)
               throw std::runtime_error("call to vkAllocateMemory failed to allocate buffer memory");

          vkBindBufferMemory(device->logical(), buffer, bufferMemory, 0);

          return std::tuple(buffer, bufferMemory);
     }

     Buffer(Core* core, Device* device, CommandPool* commandPool, VkBuffer buffer, VkDeviceMemory bufferMemory, size_t n = 1)
        : core_(core)
        , device_(device)
        , commandPool_(commandPool)
        , size_(sizeof(T) * n)
        , buffer_(buffer)
        , bufferMemory_(bufferMemory) {}

  public:
     auto& get() { return buffer_; }
     static Buffer makeUniform(Core* core, Device* device, CommandPool* commandPool) {
          VkDeviceSize bufferSize     = sizeof(T);
          auto [buffer, bufferMemory] = createBuffer(core, device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
          return Buffer(core, device, commandPool, buffer, bufferMemory);
     }
     
     static Buffer makeStaging(Core* core, Device* device, CommandPool* commandPool, size_t n) {
          VkDeviceSize bufferSize     = sizeof(T) * n;
          auto [buffer, bufferMemory] = createBuffer(core, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
          return Buffer(core, device, commandPool, buffer, bufferMemory);
     }
     
     static Buffer makeVertex(Core* core, Device* device, CommandPool* commandPool, std::vector<Vertex>& vertecies) {
          VkDeviceSize bufferSize     = sizeof(Vertex) * vertecies.size();
          auto [stagingBuffer, stagingBufferMemory] = createBuffer(core, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
          auto transferBuffer = Buffer(core, device, commandPool, stagingBuffer, stagingBufferMemory, vertecies.size());
          transferBuffer.write(vertecies.data());
          auto [vertBuffer, vertBufferMemory] = createBuffer(core, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
          auto buffer = Buffer(core, device, commandPool, vertBuffer, vertBufferMemory, vertecies.size());
          buffer.copyFrom(transferBuffer);
          return buffer;
     }
     static Buffer makeIndex(Core* core, Device* device, CommandPool* commandPool, std::vector<uint32_t>& indecies) {
          VkDeviceSize bufferSize     = sizeof(uint32_t) * indecies.size();
          auto [stagingBuffer, stagingBufferMemory] = createBuffer(core, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
          auto transferBuffer = Buffer(core, device, commandPool, stagingBuffer, stagingBufferMemory, indecies.size());
          transferBuffer.write(indecies.data());
          auto [idxBuffer, idxBufferMemory] = createBuffer(core, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
          auto buffer = Buffer(core, device, commandPool, idxBuffer, idxBufferMemory, indecies.size());
          buffer.copyFrom(transferBuffer);
          return buffer;
     }

     void write(const T* data) {
          void* bufferAdress;
          vkMapMemory(device_->logical(), bufferMemory_, 0, size_, {}, &bufferAdress);
          {
               std::memcpy(bufferAdress, data, size_);
          }
          vkUnmapMemory(device_->logical(), bufferMemory_);
     }

     void copyFrom(Buffer<T>& src) {
          commandPool_->singleTimeCommand([this, &src](VkCommandBuffer commandBuffer) {
               VkBufferCopy copyRegion {
                    .srcOffset = 0,
                    .dstOffset = 0,
                    .size      = size_
               };
               vkCmdCopyBuffer(commandBuffer, src.get(), buffer_, 1, &copyRegion);
          },
             Device::QueuePriority::TRANSFER_MEDIUM);
     }

     Buffer(const Buffer& other)            = delete;
     Buffer& operator=(const Buffer& other) = delete;

     Buffer(Buffer&& other) {
          *this = std::move(other);
     }
     Buffer& operator=(Buffer&& other) {
          if (this != &other) {
               core_               = other.core_;
               device_             = other.device_;
               commandPool_        = other.commandPool_;
               size_               = other.size_;
               buffer_             = other.buffer_;
               bufferMemory_       = other.bufferMemory_;
               other.buffer_       = nullptr;
               other.bufferMemory_ = nullptr;
          }
          return *this;
     }
     ~Buffer() {
          if (bufferMemory_ != nullptr) {
               vkFreeMemory(device_->logical(), bufferMemory_, core_->allocator());
          }
          if (buffer_ != nullptr) {
               vkDestroyBuffer(device_->logical(), buffer_, core_->allocator());
          }
     }
};
