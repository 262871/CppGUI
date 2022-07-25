// #pragma once

// #include "CommandPool.hpp"
// #include "ImageResource2D.hpp"
// #include "Swapchain.hpp"

// #include <vector>

// #include "CommandPool.hpp"
// #include "Swapchain.hpp"

// #include <vector>

// class Attachments {
//   public:
//      Attachments(Core* core, Surface* surface, Device* device)
//         : surfaceCapabilities_([](Surface* surface, Device* device) {
//              VkSurfaceCapabilitiesKHR surfaceCapabilities;
//              if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physical(), surface->surfaceKHR(), &surfaceCapabilities) != VK_SUCCESS)
//                   throw std::runtime_error("call to vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed");
//              return surfaceCapabilities;
//         }(surface, device))
//         , colorAttachment_(device, core, ImageResource2D::ImageConf { .format = VK_FORMAT_B8G8R8A8_SRGB, .extent = surfaceCapabilities_.currentExtent, .mipLevels = 1, .msaa = VK_SAMPLE_COUNT_1_BIT, .tiling = VK_IMAGE_TILING_OPTIMAL, .usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, .memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT }, ImageResource2D::ViewConf { .aspect = VK_IMAGE_ASPECT_COLOR_BIT })
//         , depthAttachment_(device, core, ImageResource2D::ImageConf { .format = VK_FORMAT_D32_SFLOAT, .extent = surfaceCapabilities_.currentExtent, .mipLevels = 1, .msaa = VK_SAMPLE_COUNT_1_BIT, .tiling = VK_IMAGE_TILING_OPTIMAL, .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, .memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT }, ImageResource2D::ViewConf { .aspect = VK_IMAGE_ASPECT_DEPTH_BIT })
//         , swapchain_(Swapchain::make(surface, device, core)) {}

//      auto swapchainImageCount() { return swapchain_.imageCount(); }
//      auto swapchainImageViews() { return swapchain_.imageViews(); }
//      auto swapchainExtent() { return swapchain_.extent(); }
//      auto colorFormat() { return colorAttachment_.format(); }
//      auto colorView() { return colorAttachment_.view(); }
//      auto depthFormat() { return depthAttachment_.format(); }
//      auto depthView() { return depthAttachment_.view(); }
//      auto msaa() { return colorAttachment_.msaa(); }

//   private:
//      VkSurfaceCapabilitiesKHR surfaceCapabilities_;
//      Swapchain                swapchain_;
//      ImageResource2D          colorAttachment_;
//      ImageResource2D          depthAttachment_;
// };

// class RenderPass {
//   public:
//      void begin(size_t framebufferIndex, CommandBuffer* commandBuffer) {
//           std::vector<VkClearValue> clear_values { { .color = clearColor_ }, { .depthStencil = { depth_, stencil_ } } };

//           VkRenderPassBeginInfo render_pass_begin_info {
//                .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
//                .pNext           = nullptr,
//                .renderPass      = renderPass_,
//                .framebuffer     = framebuffers_[framebufferIndex],
//                .renderArea      = renderArea_,
//                .clearValueCount = static_cast<uint32_t>(clear_values.size()),
//                .pClearValues    = clear_values.data()
//           };
//           vkCmdBeginRenderPass(commandBuffer->get(), &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
//      }

//      void end(CommandBuffer* commandBuffer) {
//           vkCmdEndRenderPass(commandBuffer->get());
//      }

//      auto get() { return renderPass_; }

//      RenderPass(Core* core, Device* device, Attachments* attachments)
//         : device_(device)
//         , core_(core)
//         , attachments_(attachments)
//         , framebuffers_(attachments_->swapchainImageCount())
//         , renderArea_ { { 0, 0 }, attachments_->swapchainExtent() }
//         , clearColor_ { { .01f, .01f, .01f, 1.f } }
//         , depth_(1.f)
//         , stencil_(0)
//         , samples_(attachments_->msaa()) {
//           // ----------------------------------------------------------------------------------- //
//           VkAttachmentDescription colorAttachmentDescription {
//                .flags          = {},
//                .format         = attachments_->colorFormat(),
//                .samples        = samples_,
//                .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
//                .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
//                .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
//                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
//                .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
//                .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
//           };
//           VkAttachmentReference colorAttachmentReference {
//                .attachment = 0,
//                .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
//           };
//           // ----------------------------------------------------------------------------------- //
//           VkAttachmentDescription depthAttachmentDescription {
//                .flags          = {},
//                .format         = attachments_->depthFormat(),
//                .samples        = samples_,
//                .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
//                .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
//                .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
//                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
//                .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
//                .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
//           };
//           VkAttachmentReference depthAttachmentReference {
//                .attachment = 1,
//                .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
//           };
//           // ----------------------------------------------------------------------------------- //
//           VkAttachmentDescription colorResolveAttachmentDescription {
//                .flags          = {},
//                .format         = attachments_->colorFormat(),
//                .samples        = VK_SAMPLE_COUNT_1_BIT,
//                .loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
//                .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
//                .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
//                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
//                .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
//                .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
//           };
//           VkAttachmentReference colorResolveAttachmentReference {
//                .attachment = 2,
//                .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
//           };
//           // ----------------------------------------------------------------------------------- //
//           std::vector<VkAttachmentDescription> attachmentDescriptions;
//           if (samples_ != VK_SAMPLE_COUNT_1_BIT)
//                attachmentDescriptions = { colorAttachmentDescription, depthAttachmentDescription, colorResolveAttachmentDescription };
//           else
//                attachmentDescriptions = { colorAttachmentDescription, depthAttachmentDescription };
//           // ----------------------------------------------------------------------------------- //

//           VkSubpassDescription subpassDescription {
//                .flags                   = {},
//                .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
//                .inputAttachmentCount    = 0,
//                .pInputAttachments       = nullptr,
//                .colorAttachmentCount    = 1,
//                .pColorAttachments       = &colorAttachmentReference,
//                .pResolveAttachments     = samples_ != VK_SAMPLE_COUNT_1_BIT ? &colorResolveAttachmentReference : nullptr,
//                .pDepthStencilAttachment = &depthAttachmentReference,
//                .preserveAttachmentCount = 0,
//                .pPreserveAttachments    = nullptr
//           };
//           VkSubpassDependency subpassDependency {
//                .srcSubpass      = VK_SUBPASS_EXTERNAL,
//                .dstSubpass      = {},
//                .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
//                .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
//                .srcAccessMask   = {},
//                .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
//                .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
//           };

//           VkRenderPassCreateInfo renderPassCreateInfo {
//                .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
//                .pNext           = nullptr,
//                .flags           = {},
//                .attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size()),
//                .pAttachments    = attachmentDescriptions.data(),
//                .subpassCount    = 1,
//                .pSubpasses      = &subpassDescription,
//                .dependencyCount = 1,
//                .pDependencies   = &subpassDependency
//           };
//           if (vkCreateRenderPass(device_->logical(), &renderPassCreateInfo, core_->allocator(), &renderPass_) != VK_SUCCESS)
//                throw std::runtime_error("call to vkCreateRenderPass failed");

//           createFramebuffers();
//      }
//      ~RenderPass() {
//           vkDestroyRenderPass(device_->logical(), renderPass_, core_->allocator());
//           for (auto framebuffer : framebuffers_)
//                vkDestroyFramebuffer(device_->logical(), framebuffer, core_->allocator());
//      }
//      void updateFramebuffers() {
//           renderArea_ = { { 0, 0 }, attachments_->swapchainExtent() };
//           for (auto framebuffer : framebuffers_)
//                vkDestroyFramebuffer(device_->logical(), framebuffer, core_->allocator());
//           createFramebuffers();
//      }

//   private:
//      void createFramebuffers() {
//           auto swapchainImageViews = attachments_->swapchainImageViews();
//           for (size_t i = 0; i != framebuffers_.size(); ++i) {
//                std::vector<VkImageView> attachments;
//                if (samples_ != VK_SAMPLE_COUNT_1_BIT)
//                     attachments = { attachments_->colorView(), attachments_->depthView(), swapchainImageViews[i] };
//                else
//                     attachments = { swapchainImageViews[i], attachments_->depthView() };

//                VkFramebufferCreateInfo framebufferCreateInfo {
//                     .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
//                     .pNext           = nullptr,
//                     .flags           = {},
//                     .renderPass      = renderPass_,
//                     .attachmentCount = static_cast<uint32_t>(attachments.size()),
//                     .pAttachments    = attachments.data(),
//                     .width           = renderArea_.extent.width,
//                     .height          = renderArea_.extent.height,
//                     .layers          = 1
//                };
//                if (vkCreateFramebuffer(device_->logical(), &framebufferCreateInfo, core_->allocator(), &framebuffers_[i]) != VK_SUCCESS)
//                     throw std::runtime_error("call to vkCreateFramebuffer failed");
//           }
//      }

//      Device*                    device_;
//      Core*                      core_;
//      Attachments*               attachments_;
//      VkRenderPass               renderPass_;
//      std::vector<VkFramebuffer> framebuffers_;
//      VkRect2D                   renderArea_;
//      VkClearColorValue          clearColor_;
//      float                      depth_;
//      uint32_t                   stencil_;
//      VkSampleCountFlagBits      samples_;
// };

// // class RenderPass {
// //   public:
// //      void begin(size_t imageIndex, CommandBuffer* commandBuffer) {
// //           std::vector<VkClearValue> clear_values { { .color = clearColor_ }, { .depthStencil = { depth_, stencil_ } } };

// //           VkRenderPassBeginInfo render_pass_begin_info {
// //                .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
// //                .pNext           = nullptr,
// //                .renderPass      = renderPass_,
// //                .framebuffer     = framebuffers_[imageIndex],
// //                .renderArea      = renderArea_,
// //                .clearValueCount = static_cast<uint32_t>(clear_values.size()),
// //                .pClearValues    = clear_values.data()
// //           };
// //           vkCmdBeginRenderPass(commandBuffer->get(), &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
// //      }

// //      void end(CommandBuffer* commandBuffer) {
// //           vkCmdEndRenderPass(commandBuffer->get());
// //      }

// //      auto get() { return renderPass_; }

// //      RenderPass(Core* core, Device* device, ImageResource2D* depthAttachment, ImageResource2D* colorAttachment, Swapchain* swapchain)
// //         : device_(device)
// //         , core_(core)
// //         , colorAttachment_(colorAttachment)
// //         , depthAttachment_(depthAttachment)
// //         , swapchain_(swapchain)
// //         , framebuffers_(swapchain_->imageCount())
// //         , renderArea_ { { 0, 0 }, swapchain_->extent() }
// //         , clearColor_ { {.01f, .01f, .01f, 1.f} }
// //         , depth_(1.f)
// //         , stencil_(0)
// //         , samples_(VK_SAMPLE_COUNT_1_BIT) {
// //           // ----------------------------------------------------------------------------------- //
// //           VkAttachmentDescription colorAttachmentDescription {
// //                .flags          = {},
// //                .format         = swapchain_->format(),
// //                .samples        = samples_,
// //                .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
// //                .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
// //                .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
// //                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
// //                .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
// //                .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
// //           };
// //           VkAttachmentReference colorAttachmentReference {
// //                .attachment = 0,
// //                .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
// //           };
// //           // ----------------------------------------------------------------------------------- //
// //           VkAttachmentDescription depthAttachmentDescription {
// //                .flags          = {},
// //                .format         = depthAttachment_->format(),
// //                .samples        = samples_,
// //                .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
// //                .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
// //                .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
// //                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
// //                .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
// //                .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
// //           };
// //           VkAttachmentReference depthAttachmentReference {
// //                .attachment = 1,
// //                .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
// //           };
// //           // ----------------------------------------------------------------------------------- //
// //           VkAttachmentDescription colorResolveAttachmentDescription {
// //                .flags          = {},
// //                .format         = swapchain_->format(),
// //                .samples        = VK_SAMPLE_COUNT_1_BIT,
// //                .loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
// //                .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
// //                .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
// //                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
// //                .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
// //                .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
// //           };
// //           VkAttachmentReference colorResolveAttachmentReference {
// //                .attachment = 2,
// //                .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
// //           };
// //           // ----------------------------------------------------------------------------------- //
// //           std::vector<VkAttachmentDescription> attachments;
// //           if (samples_ != VK_SAMPLE_COUNT_1_BIT)
// //                attachments = { colorAttachmentDescription, depthAttachmentDescription, colorResolveAttachmentDescription };
// //           else
// //                attachments = { colorAttachmentDescription, depthAttachmentDescription };
// //           // ----------------------------------------------------------------------------------- //

// //           VkSubpassDescription subpassDescription {
// //                .flags                   = {},
// //                .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
// //                .inputAttachmentCount    = 0,
// //                .pInputAttachments       = nullptr,
// //                .colorAttachmentCount    = 1,
// //                .pColorAttachments       = &colorAttachmentReference,
// //                .pResolveAttachments     = samples_ != VK_SAMPLE_COUNT_1_BIT? &colorResolveAttachmentReference : nullptr,
// //                .pDepthStencilAttachment = &depthAttachmentReference,
// //                .preserveAttachmentCount = 0,
// //                .pPreserveAttachments    = nullptr
// //           };
// //           VkSubpassDependency subpassDependency {
// //                .srcSubpass      = VK_SUBPASS_EXTERNAL,
// //                .dstSubpass      = {},
// //                .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
// //                .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
// //                .srcAccessMask   = {},
// //                .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
// //                .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
// //           };

// //           VkRenderPassCreateInfo renderPassCreateInfo {
// //                .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
// //                .pNext           = nullptr,
// //                .flags           = {},
// //                .attachmentCount = static_cast<uint32_t>(attachments.size()),
// //                .pAttachments    = attachments.data(),
// //                .subpassCount    = 1,
// //                .pSubpasses      = &subpassDescription,
// //                .dependencyCount = 1,
// //                .pDependencies   = &subpassDependency
// //           };
// //           if (vkCreateRenderPass(device_->logical(), &renderPassCreateInfo, core_->allocator(), &renderPass_) != VK_SUCCESS)
// //                throw std::runtime_error("call to vkCreateRenderPass failed");

// //           createFramebuffers();
// //      }
// //      ~RenderPass() {
// //           vkDestroyRenderPass(device_->logical(), renderPass_, core_->allocator());
// //           for (auto framebuffer : framebuffers_)
// //                vkDestroyFramebuffer(device_->logical(), framebuffer, core_->allocator());
// //      }
// //      void updateFramebuffers() {
// //           renderArea_ = { { 0, 0 }, swapchain_->extent() };
// //           for (auto framebuffer : framebuffers_)
// //                vkDestroyFramebuffer(device_->logical(), framebuffer, core_->allocator());
// //           createFramebuffers();
// //      }

// //   private:
// //      void createFramebuffers() {
// //           auto swapchainImageViews = swapchain_->imageViews();
// //           for (size_t i = 0; i != framebuffers_.size(); ++i) {
// //                std::vector<VkImageView> attachments;
// //                if (samples_ != VK_SAMPLE_COUNT_1_BIT)
// //                     attachments = { colorAttachment_->view(), depthAttachment_->view(), swapchainImageViews[i] };
// //                else
// //                     attachments = { swapchainImageViews[i], depthAttachment_->view()};

// //                VkFramebufferCreateInfo framebufferCreateInfo {
// //                     .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
// //                     .pNext           = nullptr,
// //                     .flags           = {},
// //                     .renderPass      = renderPass_,
// //                     .attachmentCount = static_cast<uint32_t>(attachments.size()),
// //                     .pAttachments    = attachments.data(),
// //                     .width           = renderArea_.extent.width,
// //                     .height          = renderArea_.extent.height,
// //                     .layers          = 1
// //                };
// //                if (vkCreateFramebuffer(device_->logical(), &framebufferCreateInfo, core_->allocator(), &framebuffers_[i]) != VK_SUCCESS)
// //                     throw std::runtime_error("call to vkCreateFramebuffer failed");
// //           }
// //      }

// //      Device*                    device_;
// //      Core*                      core_;
// //      ImageResource2D* colorAttachment_;
// //      ImageResource2D* depthAttachment_;
// //      Swapchain*                 swapchain_;
// //      VkRenderPass               renderPass_;
// //      std::vector<VkFramebuffer> framebuffers_;
// //      VkRect2D                   renderArea_;
// //      VkClearColorValue          clearColor_;
// //      float                      depth_;
// //      uint32_t                   stencil_;
// //      VkSampleCountFlagBits      samples_;
// // };
