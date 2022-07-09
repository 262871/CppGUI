#pragma once

#include "core.hpp"
#include "device.hpp"
#include "swapchain.hpp"

#include <volk.h>

#include <vector>

class command_buffer {
     enum state {
          NOT_ALLOCATED,
          READY,
          RECORDING,
          RENDERING,
          ENDED,
          SUBMITTED
     };

     VkCommandBuffer command_buffer_;
     state           state_;
};

class render_pass {
  public:
     enum state {
          NOT_ALLOCATED,
          READY,
          BEGINNING,
          ONGOING,
          ENDED,
          SUBMITTED
     };

     render_pass(device* gpu, core* vulkan, swapchain* target_swapchain)
        : device_(gpu)
        , vulkan_core_(vulkan) {
          // ----------------------------------------------------------------------------------- //
          VkAttachmentDescription color_attachment {
               .flags          = {},
               .format         = target_swapchain->format(),
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
               .format         = target_swapchain->depth_format(),
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
          //      .format         = target_swapchain->format(),
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
          std::vector attachments { color_attachment, depth_attachment/* , color_attachment_resolve */ };
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
          if (vkCreateRenderPass(device_->logical(), &render_pass_create_info, vulkan_core_->allocator(), &render_pass_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateRenderPass failed");
     }
     ~render_pass() {
          vkDestroyRenderPass(device_->logical(), render_pass_, vulkan_core_->allocator());
     }

  private:
     device*           device_;
     core*             vulkan_core_;
     VkRenderPass      render_pass_;
     // VkRect2D          render_area_;
     // VkClearColorValue clear_color_;
     // float             depth_;
     // uint32_t          stencil_;
     // state             state_;
};
