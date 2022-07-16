#pragma once

#include "CommandPool.hpp"
#include "Swapchain.hpp"

#include <vector>

class RenderPass {
  public:
     void begin(size_t framebufferIndex, CommandBuffer* commandBuffer) {
          std::vector<VkClearValue> clear_values { { .color = clearColor_ }, { .depthStencil = { depth_, stencil_ } } };

          VkRenderPassBeginInfo render_pass_begin_info {
               .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
               .pNext           = nullptr,
               .renderPass      = renderPass_,
               .framebuffer     = framebuffers_[framebufferIndex],
               .renderArea      = renderArea_,
               .clearValueCount = static_cast<uint32_t>(clear_values.size()),
               .pClearValues    = clear_values.data()
          };
          vkCmdBeginRenderPass(commandBuffer->get(), &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
     }

     void end(CommandBuffer* commandBuffer) {
          vkCmdEndRenderPass(commandBuffer->get());
     }
     
     auto get() { return renderPass_; } 

     RenderPass(Device* device, Core* core, Swapchain* swapchain)
        : device_(device)
        , core_(core)
        , swapchain_(swapchain)
        , framebuffers_(swapchain_->imageCount())
        , renderArea_ { { 0, 0 }, swapchain_->extent() }
        , clearColor_ { .01f, .01f, .01f, 1.f }
        , depth_(1.f)
        , stencil_(0)
        , samples_(VK_SAMPLE_COUNT_4_BIT) {
          // ----------------------------------------------------------------------------------- //
          VkAttachmentDescription color_attachment {
               .flags          = {},
               .format         = swapchain_->format(),
               .samples        = samples_,
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
               .format         = swapchain_->depthFormat(),
               .samples        = samples_,
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
          VkAttachmentDescription color_attachment_resolve {
               .flags          = {},
               .format         = swapchain_->format(),
               .samples        = VK_SAMPLE_COUNT_1_BIT,
               .loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
               .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
          };
          VkAttachmentReference color_attachment_resolve_reference {
               .attachment = 2,
               .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
          };
          // ----------------------------------------------------------------------------------- //
          std::vector attachments { color_attachment, depth_attachment, color_attachment_resolve };
          // ----------------------------------------------------------------------------------- //

          VkSubpassDescription subpass_description {
               .flags                   = {},
               .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
               .inputAttachmentCount    = 0,
               .pInputAttachments       = nullptr,
               .colorAttachmentCount    = 1,
               .pColorAttachments       = &color_attachment_reference,
               .pResolveAttachments     = &color_attachment_resolve_reference,
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
          
          createFramebuffers();
     }
     ~RenderPass() {
          fmt::print("RenderPass destructor\n");
          vkDestroyRenderPass(device_->logical(), renderPass_, core_->allocator());
          for (auto framebuffer : framebuffers_)
               vkDestroyFramebuffer(device_->logical(), framebuffer, core_->allocator());
     }

  private:
     void createFramebuffers() {;
          auto swapchainImageViews = swapchain_->imageViews();
          for (size_t i = 0; i != framebuffers_.size(); ++i) {
               std::vector attachments { swapchain_->colorView(), swapchain_->depthView(), swapchainImageViews[i] };

               VkFramebufferCreateInfo vkFramebufferCreateInfo {
                    .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .pNext           = nullptr,
                    .flags           = {},
                    .renderPass      = renderPass_,
                    .attachmentCount = static_cast<uint32_t>(attachments.size()),
                    .pAttachments    = attachments.data(),
                    .width           = renderArea_.extent.width,
                    .height          = renderArea_.extent.height,
                    .layers          = 1
               };
               if (vkCreateFramebuffer(device_->logical(), &vkFramebufferCreateInfo, core_->allocator(), &framebuffers_[i]) != VK_SUCCESS)
                    throw std::runtime_error("call to vkCreateFramebuffer failed");
          }
     }

     Device*                    device_;
     Core*                      core_;
     Swapchain*                 swapchain_;
     VkRenderPass               renderPass_;
     std::vector<VkFramebuffer> framebuffers_;
     VkRect2D                   renderArea_;
     VkClearColorValue          clearColor_;
     float                      depth_;
     uint32_t                   stencil_;
     VkSampleCountFlagBits      samples_;
};
