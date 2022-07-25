#pragma once

#include "DescriptorSets.hpp"
#include "ImageResource2D.hpp"
#include "Swapchain.hpp"

class RenderProgram {
  public:
     struct Attachments {
          Swapchain*       swapchain;
          ImageResource2D* colorbuffer;
          ImageResource2D* depthbuffer;
     };
     RenderProgram(Core* core, Device* device, DescriptorSetLayout* descriptorSetLayout, Attachments attachments)
        : core_(core)
        , device_(device)
        , descriptorSetLayout_(descriptorSetLayout)
        , attachments_(attachments) {
          createRenderPass();
          createFramebuffers();
          createPipelineLayout();
          createPipeline();
     }
     ~RenderProgram() {
          vkDestroyRenderPass(device_->logical(), renderPass_, core_->allocator());
          for (auto framebuffer : framebuffers_)
               vkDestroyFramebuffer(device_->logical(), framebuffer, core_->allocator());
          vkDestroyPipeline(device_->logical(), pipeline_, core_->allocator());
          vkDestroyPipelineLayout(device_->logical(), pipelineLayout_, core_->allocator());
     }
     void beginRenderPass(size_t framebufferIndex, CommandBuffer* commandBuffer) {
          std::vector<VkClearValue> clear_values { { .color = clearColor_ }, { .depthStencil = { depth_, stencil_ } } };
          VkRenderPassBeginInfo     renderPassBeginInfo {
                   .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                   .pNext           = nullptr,
                   .renderPass      = renderPass_,
                   .framebuffer     = framebuffers_[framebufferIndex],
                   .renderArea      = { { 0, 0 }, attachments_.swapchain->extent() },
                   .clearValueCount = static_cast<uint32_t>(clear_values.size()),
                   .pClearValues    = clear_values.data()
          };
          vkCmdBeginRenderPass(commandBuffer->get(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
     }
     void endRenderPass(CommandBuffer* commandBuffer) {
          vkCmdEndRenderPass(commandBuffer->get());
     }
     void resize() {
          for (auto framebuffer : framebuffers_)
               vkDestroyFramebuffer(device_->logical(), framebuffer, core_->allocator());
          createFramebuffers();
     }
     auto& pipeline() { return pipeline_; }
     auto& pipelineLayout() { return pipelineLayout_; }

  private:
     void createRenderPass() {
          VkAttachmentDescription colorAttachmentDescription {
               .flags          = {},
               .format         = attachments_.colorbuffer->format(),
               .samples        = attachments_.colorbuffer->msaa(),
               .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
               .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
          };
          VkAttachmentReference colorAttachmentReference {
               .attachment = 0,
               .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
          };
          // ----------------------------------------------------------------------------------- //
          VkAttachmentDescription depthAttachmentDescription {
               .flags          = {},
               .format         = attachments_.depthbuffer->format(),
               .samples        = attachments_.depthbuffer->msaa(),
               .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
               .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
               .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
          };
          VkAttachmentReference depthAttachmentReference {
               .attachment = 1,
               .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
          };
          // ----------------------------------------------------------------------------------- //
          VkAttachmentDescription colorResolveAttachmentDescription {
               .flags          = {},
               .format         = attachments_.swapchain->format(),
               .samples        = VK_SAMPLE_COUNT_1_BIT,
               .loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
               .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
               .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
               .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
               .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
          };
          VkAttachmentReference colorResolveAttachmentReference {
               .attachment = 2,
               .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
          };
          // ----------------------------------------------------------------------------------- //
          std::vector<VkAttachmentDescription> attachmentDescriptions { colorAttachmentDescription, depthAttachmentDescription, colorResolveAttachmentDescription };
          // ----------------------------------------------------------------------------------- //
          VkSubpassDescription subpassDescription {
               .flags                   = {},
               .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
               .inputAttachmentCount    = 0,
               .pInputAttachments       = nullptr,
               .colorAttachmentCount    = 1,
               .pColorAttachments       = &colorAttachmentReference,
               .pResolveAttachments     = &colorResolveAttachmentReference,
               .pDepthStencilAttachment = &depthAttachmentReference,
               .preserveAttachmentCount = 0,
               .pPreserveAttachments    = nullptr
          };
          VkSubpassDependency subpassDependency {
               .srcSubpass      = VK_SUBPASS_EXTERNAL,
               .dstSubpass      = {},
               .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
               .srcAccessMask   = {},
               .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
               .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
          };
          // ----------------------------------------------------------------------------------- //
          VkRenderPassCreateInfo renderPassCreateInfo {
               .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
               .pNext           = nullptr,
               .flags           = {},
               .attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size()),
               .pAttachments    = attachmentDescriptions.data(),
               .subpassCount    = 1,
               .pSubpasses      = &subpassDescription,
               .dependencyCount = 1,
               .pDependencies   = &subpassDependency
          };
          if (vkCreateRenderPass(device_->logical(), &renderPassCreateInfo, core_->allocator(), &renderPass_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateRenderPass failed");
     }
     // ---------------------------------------------------------------------------------------- //
     // ---------------------------------------------------------------------------------------- //
     void createFramebuffers() {
          auto swapchainImageViews = attachments_.swapchain->imageViews();
          framebuffers_.resize(swapchainImageViews.size());
          for (size_t i = 0; i != framebuffers_.size(); ++i) {
               std::vector<VkImageView> attachments { attachments_.colorbuffer->view(), attachments_.depthbuffer->view(), swapchainImageViews[i] };
               VkFramebufferCreateInfo  framebufferCreateInfo {
                     .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                     .pNext           = nullptr,
                     .flags           = {},
                     .renderPass      = renderPass_,
                     .attachmentCount = static_cast<uint32_t>(attachments.size()),
                     .pAttachments    = attachments.data(),
                     .width           = attachments_.swapchain->extent().width,
                     .height          = attachments_.swapchain->extent().height,
                     .layers          = 1
               };
               if (vkCreateFramebuffer(device_->logical(), &framebufferCreateInfo, core_->allocator(), &framebuffers_[i]) != VK_SUCCESS)
                    throw std::runtime_error("call to vkCreateFramebuffer failed");
          }
     }
     // ---------------------------------------------------------------------------------------- //
     // ---------------------------------------------------------------------------------------- //
     void createPipelineLayout() {
          VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo {
               .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
               .pNext                  = nullptr,
               .flags                  = {},
               .setLayoutCount         = 1,
               .pSetLayouts            = &descriptorSetLayout_->get(),
               .pushConstantRangeCount = 0,
               .pPushConstantRanges    = nullptr
          };
          if (vkCreatePipelineLayout(device_->logical(), &vkPipelineLayoutCreateInfo, core_->allocator(), &pipelineLayout_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreatePipelineLayout failed");
     }
     // ---------------------------------------------------------------------------------------- //
     // ---------------------------------------------------------------------------------------- //
     static std::vector<char> readFile(const std::string& filename) {
          std::ifstream file(filename, std::ios::ate | std::ios::binary);
          if (!file.is_open())
               throw std::runtime_error("failed to open file!");

          size_t            fileSize = (size_t)file.tellg();
          std::vector<char> buffer(fileSize);

          file.seekg(0);
          file.read(buffer.data(), fileSize);
          file.close();

          return buffer;
     }
     // ---------------------------------------------------------------------------------------- //
     // ---------------------------------------------------------------------------------------- //
     VkShaderModule createShaderModule(const std::vector<char>& shaderCode) {
          VkShaderModuleCreateInfo vertShaderModuleCreateInfo {
               .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
               .pNext    = nullptr,
               .flags    = {},
               .codeSize = static_cast<uint32_t>(shaderCode.size()),
               .pCode    = reinterpret_cast<const uint32_t*>(shaderCode.data())
          };
          VkShaderModule shaderModule;
          if (vkCreateShaderModule(device_->logical(), &vertShaderModuleCreateInfo, core_->allocator(), &shaderModule) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateShaderModule failed");
          return shaderModule;
     }
     // ---------------------------------------------------------------------------------------- //
     // ---------------------------------------------------------------------------------------- //
     void createPipeline() {
          std::vector<char> vertShaderCode   = readFile("./shaders/shader.vert.spv");
          VkShaderModule    vertShaderModule = createShaderModule(vertShaderCode);
          std::vector<char> fragShaderCode   = readFile("./shaders/shader.frag.spv");
          VkShaderModule    fragShaderModule = createShaderModule(fragShaderCode);

          std::vector<VkPipelineShaderStageCreateInfo> shaderStages {
               VkPipelineShaderStageCreateInfo {
                  .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                  .pNext               = nullptr,
                  .flags               = {},
                  .stage               = VK_SHADER_STAGE_VERTEX_BIT,
                  .module              = vertShaderModule,
                  .pName               = "main",
                  .pSpecializationInfo = nullptr },
               VkPipelineShaderStageCreateInfo {
                  .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                  .pNext               = nullptr,
                  .flags               = {},
                  .stage               = VK_SHADER_STAGE_FRAGMENT_BIT,
                  .module              = fragShaderModule,
                  .pName               = "main",
                  .pSpecializationInfo = nullptr }
          };
          // ----------------------------------------------------------------------------------- //
          auto bindingDescription    = Vertex::bindingDescription();
          auto attributeDescriptions = Vertex::attributeDescriptions();

          VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo {
               .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
               .pNext                           = nullptr,
               .flags                           = {},
               .vertexBindingDescriptionCount   = 1,
               .pVertexBindingDescriptions      = &bindingDescription,
               .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
               .pVertexAttributeDescriptions    = attributeDescriptions.data()
          };
          VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo {
               .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
               .pNext                  = nullptr,
               .flags                  = {},
               .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
               .primitiveRestartEnable = VK_FALSE
          };
          VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo {
               .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
               .pNext         = nullptr,
               .flags         = {},
               .viewportCount = 1,
               .pViewports    = nullptr,
               .scissorCount  = 1,
               .pScissors     = nullptr
          };
          VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo {
               .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
               .pNext                   = nullptr,
               .flags                   = {},
               .depthClampEnable        = VK_FALSE,
               .rasterizerDiscardEnable = VK_FALSE,
               .polygonMode             = VK_POLYGON_MODE_FILL,
               .cullMode                = VK_CULL_MODE_BACK_BIT,
               .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
               .depthBiasEnable         = VK_FALSE,
               .depthBiasConstantFactor = 0.f,
               .depthBiasClamp          = 0.f,
               .depthBiasSlopeFactor    = 0.f,
               .lineWidth               = 1.f
          };
          VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo {
               .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
               .pNext                 = nullptr,
               .flags                 = {},
               .rasterizationSamples  = attachments_.depthbuffer->msaa(),
               .sampleShadingEnable   = VK_TRUE,
               .minSampleShading      = 0.5f,
               .pSampleMask           = nullptr,
               .alphaToCoverageEnable = VK_FALSE,
               .alphaToOneEnable      = VK_FALSE
          };
          VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState {
               .blendEnable         = VK_TRUE,
               .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
               .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
               .colorBlendOp        = VK_BLEND_OP_ADD,
               .srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
               .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
               .alphaBlendOp        = VK_BLEND_OP_ADD,
               .colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
          };
          VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo {
               .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
               .pNext                 = nullptr,
               .flags                 = {},
               .depthTestEnable       = VK_TRUE,
               .depthWriteEnable      = VK_TRUE,
               .depthCompareOp        = VK_COMPARE_OP_LESS,
               .depthBoundsTestEnable = VK_FALSE,
               .stencilTestEnable     = VK_FALSE,
               .front                 = {},
               .back                  = {},
               .minDepthBounds        = 0.f,
               .maxDepthBounds        = 1.f
          };
          VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo {
               .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
               .pNext           = nullptr,
               .flags           = {},
               .logicOpEnable   = VK_FALSE,
               .logicOp         = VK_LOGIC_OP_COPY,
               .attachmentCount = 1,
               .pAttachments    = &vkPipelineColorBlendAttachmentState,
               .blendConstants  = { 0.f, 0.f, 0.f, 0.f }
          };
          // ----------------------------------------------------------------------------------- //
          std::vector<VkDynamicState> dynamicStates = {
               VK_DYNAMIC_STATE_VIEWPORT,
               VK_DYNAMIC_STATE_SCISSOR
          };
          VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo {
               .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
               .pNext             = nullptr,
               .flags             = {},
               .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
               .pDynamicStates    = dynamicStates.data(),
          };
          // ----------------------------------------------------------------------------------- //
          VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo {
               .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
               .pNext               = nullptr,
               .flags               = {},
               .stageCount          = static_cast<uint32_t>(shaderStages.size()),
               .pStages             = shaderStages.data(),
               .pVertexInputState   = &vkPipelineVertexInputStateCreateInfo,
               .pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo,
               .pTessellationState  = nullptr,
               .pViewportState      = &vkPipelineViewportStateCreateInfo,
               .pRasterizationState = &vkPipelineRasterizationStateCreateInfo,
               .pMultisampleState   = &vkPipelineMultisampleStateCreateInfo,
               .pDepthStencilState  = &vkPipelineDepthStencilStateCreateInfo,
               .pColorBlendState    = &vkPipelineColorBlendStateCreateInfo,
               .pDynamicState       = &pipelineDynamicStateCreateInfo,
               .layout              = pipelineLayout_,
               .renderPass          = renderPass_,
               .subpass             = 0u,
               .basePipelineHandle  = {},
               .basePipelineIndex   = 0
          };
          // ----------------------------------------------------------------------------------- //
          if (vkCreateGraphicsPipelines(device_->logical(), {}, 1u, &vkGraphicsPipelineCreateInfo, core_->allocator(), &pipeline_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateGraphicsPipelines failed");
          // ----------------------------------------------------------------------------------- //
          vkDestroyShaderModule(device_->logical(), vertShaderModule, core_->allocator());
          vkDestroyShaderModule(device_->logical(), fragShaderModule, core_->allocator());
     }
     // ---------------------------------------------------------------------------------------- //
     VkClearColorValue clearColor_ { { .01f, .01f, .01f, 1.f } };
     float             depth_ { 1.f };
     uint32_t          stencil_ { 0 };

     Core*                core_;
     Device*              device_;
     DescriptorSetLayout* descriptorSetLayout_;
     Attachments          attachments_;

     VkRenderPass               renderPass_;
     std::vector<VkFramebuffer> framebuffers_;
     VkPipelineLayout           pipelineLayout_;
     VkPipeline                 pipeline_;
};
