#pragma once

#include "Core.hpp"
#include "Device.hpp"

#include <functional>

class CommandBuffer {
  public:
     auto get() { return commandBuffer_; }
     void begin() {
          VkCommandBufferBeginInfo vkCommandBufferBeginInfo {
               .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
               .pNext            = nullptr,
               .flags            = {},
               .pInheritanceInfo = nullptr
          };

          if (vkBeginCommandBuffer(commandBuffer_, &vkCommandBufferBeginInfo) != VK_SUCCESS)
               throw std::runtime_error("call to vkBeginCommandBuffer failed");
     }
     void end() {
          if (vkEndCommandBuffer(commandBuffer_) != VK_SUCCESS)
               throw std::runtime_error("call to vkEndCommandBuffer failed");
     }

  private:
     friend class CommandPool;
     CommandBuffer(VkCommandBuffer commandBuffer)
        : commandBuffer_(commandBuffer) {
     }
     VkCommandBuffer commandBuffer_;
};
class CommandPool {
  public:
     CommandPool(Device* device, Core* core, uint32_t queueFamilyIndex)
        : device_(device)
        , core_(core) {
          VkCommandPoolCreateInfo command_pool_create_info {
               .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
               .pNext            = nullptr,
               .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
               .queueFamilyIndex = queueFamilyIndex
          };
          if (vkCreateCommandPool(device_->logical(), &command_pool_create_info, core_->allocator(), &commandPool_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateCommandPool failed");
     }
     ~CommandPool() {
          vkDestroyCommandPool(device_->logical(), commandPool_, core_->allocator());
     }

     void singleTimeCommand(std::function<void(VkCommandBuffer)> callback) {
          VkCommandBufferAllocateInfo command_buffer_allocate_info {
               .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
               .pNext              = nullptr,
               .commandPool        = commandPool_,
               .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
               .commandBufferCount = 1
          };
          VkCommandBuffer command_buffer;
          if (vkAllocateCommandBuffers(device_->logical(), &command_buffer_allocate_info, &command_buffer) != VK_SUCCESS)
               throw std::runtime_error("call to vkAllocateCommandBuffers failed");
          VkCommandBufferBeginInfo command_buffer_begin_info {
               .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
               .pNext            = nullptr,
               .flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
               .pInheritanceInfo = nullptr
          };
          if (vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info) != VK_SUCCESS)
               throw std::runtime_error("call to vkBeginCommandBuffer failed");

          callback(command_buffer);

          vkEndCommandBuffer(command_buffer);
          VkSubmitInfo submit_info {
               .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
               .pNext                = nullptr,
               .waitSemaphoreCount   = 0,
               .pWaitSemaphores      = nullptr,
               .pWaitDstStageMask    = nullptr,
               .commandBufferCount   = 1,
               .pCommandBuffers      = &command_buffer,
               .signalSemaphoreCount = 0,
               .pSignalSemaphores    = nullptr
          };
          if (vkQueueSubmit(device_->graphics(), 1, &submit_info, nullptr) != VK_SUCCESS)
               throw std::runtime_error("call to vkQueueSubmit failed");
          if (vkQueueWaitIdle(device_->graphics()) != VK_SUCCESS)
               throw std::runtime_error("call to vkQueueWaitIdle failed");
          vkFreeCommandBuffers(device_->logical(), commandPool_, 1, &command_buffer);
     }

     std::vector<CommandBuffer> createCommandBuffers(size_t size) {
          auto                        vkCommandBuffers = std::vector<VkCommandBuffer>(size);
          VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo {
               .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
               .pNext              = nullptr,
               .commandPool        = commandPool_,
               .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
               .commandBufferCount = static_cast<uint32_t>(size)
          };
          if (vkAllocateCommandBuffers(device_->logical(), &vkCommandBufferAllocateInfo, vkCommandBuffers.data()) != VK_SUCCESS)
               throw std::runtime_error("call to vkAllocateCommandBuffers failed");
          std::vector<CommandBuffer> commandBuffers;
          for (auto vkCB : commandBuffers)
               commandBuffers.emplace_back(vkCB);
          return commandBuffers;
     }

     CommandBuffer createCommandBuffer() {
          VkCommandBuffer             vkCommandBuffer;
          VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo {
               .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
               .pNext              = nullptr,
               .commandPool        = commandPool_,
               .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
               .commandBufferCount = 1
          };
          if (vkAllocateCommandBuffers(device_->logical(), &vkCommandBufferAllocateInfo, &vkCommandBuffer) != VK_SUCCESS)
               throw std::runtime_error("call to vkAllocateCommandBuffers failed");
          return vkCommandBuffer;
     }

  private:
     Device*       device_;
     Core*         core_;
     VkCommandPool commandPool_;
};


