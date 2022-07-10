#pragma once

#include "Core.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"

#include <vector>

// void create_command_pool() {
//      VkCommandPoolCreateInfo command_pool_create_info {
//           .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
//           .pNext            = nullptr,
//           .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
//           .queueFamilyIndex = queue_family_indices_.front()
//      };
//      if (vkCreateCommandPool(device_, &command_pool_create_info, nullptr, &command_pool_) != VK_SUCCESS)
//           throw std::runtime_error("call to vkCreateCommandPool failed");
// }

// VkCommandBuffer begin_single_time_commands() {
//      VkCommandBufferAllocateInfo command_buffer_allocate_info {
//           .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
//           .pNext              = nullptr,
//           .commandPool        = command_pool_,
//           .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
//           .commandBufferCount = 1
//      };

//      VkCommandBuffer command_buffer;
//      if (vkAllocateCommandBuffers(device_, &command_buffer_allocate_info, &command_buffer) != VK_SUCCESS)
//           throw std::runtime_error("call to vkAllocateCommandBuffers failed");

//      VkCommandBufferBeginInfo command_buffer_begin_info {
//           .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
//           .pNext            = nullptr,
//           .flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
//           .pInheritanceInfo = nullptr
//      };

//      if (vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info) != VK_SUCCESS)
//           throw std::runtime_error("call to vkBeginCommandBuffer failed");

//      return command_buffer;
// }

// void end_single_time_commands(VkCommandBuffer command_buffer) {
//      vkEndCommandBuffer(command_buffer);
//      VkSubmitInfo submit_info {
//           .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
//           .pNext                = nullptr,
//           .waitSemaphoreCount   = 0,
//           .pWaitSemaphores      = nullptr,
//           .pWaitDstStageMask    = nullptr,
//           .commandBufferCount   = 1,
//           .pCommandBuffers      = &command_buffer,
//           .signalSemaphoreCount = 0,
//           .pSignalSemaphores    = nullptr
//      };

//      if (vkQueueSubmit(graphics_queue_, 1, &submit_info, nullptr) != VK_SUCCESS)
//           throw std::runtime_error("call to vkQueueSubmit failed");
//      if (vkQueueWaitIdle(graphics_queue_) != VK_SUCCESS)
//           throw std::runtime_error("call to vkQueueWaitIdle failed");
//      vkFreeCommandBuffers(device_, command_pool_, 1, &command_buffer);
// }

class CommandBuffer {
  public:
     auto get() { return commandBuffer_; }

  private:
     VkCommandBuffer commandBuffer_;
};

class FrameBuffer {
  public:
     auto get() { return frameBuffer_; }

  private:
     VkFramebuffer frameBuffer_;
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
     
     void singleTimeCommand(auto callback) {
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

  private:
     Device*       device_;
     Core*         core_;
     VkCommandPool commandPool_;
};

class RenderPass {
  public:
     void begin(FrameBuffer* frameBuffer, CommandBuffer* commandBuffer) {
          std::vector<VkClearValue> clear_values { { .color = clearColor_ }, { .depthStencil = { depth_, stencil_ } } };

          VkRenderPassBeginInfo render_pass_begin_info {
               .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
               .pNext           = nullptr,
               .renderPass      = renderPass_,
               .framebuffer     = frameBuffer->get(),
               .renderArea      = renderArea_,
               .clearValueCount = static_cast<uint32_t>(clear_values.size()),
               .pClearValues    = clear_values.data()
          };
          vkCmdBeginRenderPass(commandBuffer->get(), &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
     }

     void end(CommandBuffer* commandBuffer) {
          vkCmdEndRenderPass(commandBuffer->get());
     }

     RenderPass(Device* device, Core* core, Swapchain* swapchain)
        : device_(device)
        , core_(core)
        , renderArea_ { { 0, 0 }, swapchain->extent() }
        , clearColor_ { .01f, .01f, .01f, 1.f }
        , depth_(1.f)
        , stencil_(0) {
          // ----------------------------------------------------------------------------------- //
          VkAttachmentDescription color_attachment {
               .flags          = {},
               .format         = swapchain->format(),
               .samples        = VK_SAMPLE_COUNT_1_BIT,
               .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
               .finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
          };
          VkAttachmentReference color_attachment_reference {
               .attachment = 0,
               .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
          };
          // ----------------------------------------------------------------------------------- //
          VkAttachmentDescription depth_attachment {
               .flags          = {},
               .format         = swapchain->depthFormat(),
               .samples        = VK_SAMPLE_COUNT_1_BIT,
               .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
               .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
               .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
          };
          VkAttachmentReference depth_attachment_reference {
               .attachment = 1,
               .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
          };
          // ----------------------------------------------------------------------------------- //
          // VkAttachmentDescription color_attachment_resolve {
          //      .flags          = {},
          //      .format         = swapchain->format(),
          //      .samples        = VK_SAMPLE_COUNT_1_BIT,
          //      .loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
          //      .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
          //      .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
          //      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
          //      .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
          //      .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
          // };
          // VkAttachmentReference color_attachment_resolve_reference {
          //      .attachment = 2,
          //      .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
          // };
          // ----------------------------------------------------------------------------------- //
          std::vector attachments { color_attachment, depth_attachment /* , color_attachment_resolve */ };
          // ----------------------------------------------------------------------------------- //

          VkSubpassDescription subpass_description {
               .flags                   = {},
               .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
               .inputAttachmentCount    = 0,
               .pInputAttachments       = nullptr,
               .colorAttachmentCount    = 1,
               .pColorAttachments       = &color_attachment_reference,
               .pResolveAttachments     = nullptr /* &color_attachment_resolve_reference */,
               .pDepthStencilAttachment = &depth_attachment_reference,
               .preserveAttachmentCount = 0,
               .pPreserveAttachments    = nullptr
          };

          VkSubpassDependency subpass_dependency {
               .srcSubpass      = VK_SUBPASS_EXTERNAL,
               .dstSubpass      = {},
               .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .srcAccessMask   = {},
               .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
          };

          VkRenderPassCreateInfo render_pass_create_info {
               .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
               .pNext           = nullptr,
               .flags           = {},
               .attachmentCount = static_cast<uint32_t>(attachments.size()),
               .pAttachments    = attachments.data(),
               .subpassCount    = 1,
               .pSubpasses      = &subpass_description,
               .dependencyCount = 1,
               .pDependencies   = &subpass_dependency
          };
          if (vkCreateRenderPass(device_->logical(), &render_pass_create_info, core_->allocator(), &renderPass_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateRenderPass failed");
     }
     ~RenderPass() {
          vkDestroyRenderPass(device_->logical(), renderPass_, core_->allocator());
     }

  private:
     Device*           device_;
     Core*             core_;
     VkRenderPass      renderPass_;
     VkRect2D          renderArea_;
     VkClearColorValue clearColor_;
     float             depth_;
     uint32_t          stencil_;
};
