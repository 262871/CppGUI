#pragma once

#include "Core.hpp"
#include "Device.hpp"

#include <functional>

class CommandPool;

class CommandBuffer {
  public:
     VkCommandBuffer& get();
     void begin();
     void end();
     CommandBuffer(VkCommandBuffer commandBuffer, CommandPool* commandPool);
     ~CommandBuffer();
     
     CommandBuffer(const CommandBuffer& other) = delete;
     CommandBuffer& operator=(const CommandBuffer& other) = delete;
     
     CommandBuffer(CommandBuffer&& other) {
          commandPool_ = other.commandPool_;
          commandBuffer_ = other.commandBuffer_;
          other.commandBuffer_ = nullptr;
     }
     CommandBuffer& operator=(CommandBuffer&& other) {
          commandPool_ = other.commandPool_;
          commandBuffer_ = other.commandBuffer_;
          other.commandBuffer_ = nullptr;
          return *this;
     }

  private:
     CommandPool* commandPool_;
     VkCommandBuffer commandBuffer_;
};

class CommandPool {
     auto chooseQueue(Device::QueuePriority queue) {
          if (queue == Device::QueuePriority::TRANSFER_REALTIME)
               return device_->transfer();
          return device_->graphics();
     }
  public:
     CommandPool(Core* core, Device* device)
        : core_(core)
        , device_(device) {
          VkCommandPoolCreateInfo commandPoolCreateInfo {
               .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
               .pNext            = nullptr,
               .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
               .queueFamilyIndex = 0
          };
          if (vkCreateCommandPool(device_->logical(), &commandPoolCreateInfo, core_->allocator(), &commandPool_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateCommandPool failed");
     }
     ~CommandPool() {
          vkDestroyCommandPool(device_->logical(), commandPool_, core_->allocator());
     }

     void singleTimeCommand(std::function<void(VkCommandBuffer)> callback, Device::QueuePriority queue) {
          VkCommandBufferAllocateInfo commandBufferAllocateInfo {
               .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
               .pNext              = nullptr,
               .commandPool        = commandPool_,
               .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
               .commandBufferCount = 1
          };
          VkCommandBuffer commandBuffer;
          if (vkAllocateCommandBuffers(device_->logical(), &commandBufferAllocateInfo, &commandBuffer) != VK_SUCCESS)
               throw std::runtime_error("call to vkAllocateCommandBuffers failed");
          VkCommandBufferBeginInfo commandBufferBeginInfo {
               .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
               .pNext            = nullptr,
               .flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
               .pInheritanceInfo = nullptr
          };
          if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS)
               throw std::runtime_error("call to vkBeginCommandBuffer failed");

          callback(commandBuffer);

          vkEndCommandBuffer(commandBuffer);
          VkSubmitInfo submitInfo {
               .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
               .pNext                = nullptr,
               .waitSemaphoreCount   = 0,
               .pWaitSemaphores      = nullptr,
               .pWaitDstStageMask    = nullptr,
               .commandBufferCount   = 1,
               .pCommandBuffers      = &commandBuffer,
               .signalSemaphoreCount = 0,
               .pSignalSemaphores    = nullptr
          };
          if (vkQueueSubmit(chooseQueue(queue), 1, &submitInfo, nullptr) != VK_SUCCESS)
               throw std::runtime_error("call to vkQueueSubmit failed");
          if (vkQueueWaitIdle(chooseQueue(queue)) != VK_SUCCESS)
               throw std::runtime_error("call to vkQueueWaitIdle failed");
          vkFreeCommandBuffers(device_->logical(), commandPool_, 1, &commandBuffer);
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
          for (auto vkCB : vkCommandBuffers)
               commandBuffers.push_back(CommandBuffer(vkCB, this));
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
          return CommandBuffer(vkCommandBuffer, this);
     }
     
     void free(CommandBuffer* commandBuffer) {
          vkFreeCommandBuffers(device_->logical(), commandPool_, 1, &commandBuffer->get());
     }

  private:
     Core*         core_;
     Device*       device_;
     VkCommandPool commandPool_;
};



VkCommandBuffer& CommandBuffer::get() { return commandBuffer_; }
void CommandBuffer::begin() {
     VkCommandBufferBeginInfo vkCommandBufferBeginInfo {
          .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
          .pNext            = nullptr,
          .flags            = {},
          .pInheritanceInfo = nullptr
     };

     if (vkBeginCommandBuffer(commandBuffer_, &vkCommandBufferBeginInfo) != VK_SUCCESS)
          throw std::runtime_error("call to vkBeginCommandBuffer failed");
}
void CommandBuffer::end() {
     if (vkEndCommandBuffer(commandBuffer_) != VK_SUCCESS)
          throw std::runtime_error("call to vkEndCommandBuffer failed");
}
CommandBuffer::~CommandBuffer() {
     if (commandBuffer_)
          commandPool_->free(this);
}

CommandBuffer::CommandBuffer(VkCommandBuffer commandBuffer, CommandPool* commandPool)
     : commandPool_(commandPool)
     , commandBuffer_(commandBuffer) {
}

