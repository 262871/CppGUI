#pragma once

#include "CommandPool.hpp"
#include "Core.hpp"
#include "Data.hpp"
#include "DescriptorSets.hpp"
#include "Device.hpp"
#include "RenderPass.hpp"
#include "Swapchain.hpp"

#include <fstream>
#include <tuple>

class GraphicsPipeline {
  public:
     GraphicsPipeline(Core* core, Device* device, Swapchain* swapchain, RenderPass* renderPass, DescriptorSetLayout* descriptorSetLayout)
     : core_(core)
     , device_(device)
     , swapchain_(swapchain)
     , renderPass_(renderPass)
     , descriptorSetLayout_(descriptorSetLayout) {
          std::vector<uint32_t>    vertShaderCode = formattedSPIRV("./shaders/shader.vert.spv");
          VkShaderModuleCreateInfo vertShaderModuleCreateInfo {
               .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
               .pNext    = nullptr,
               .flags    = {},
               .codeSize = static_cast<uint32_t>(vertShaderCode.size()) * 4,
               .pCode    = vertShaderCode.data()
          };
          VkShaderModule vertShaderModule {};
          if (vkCreateShaderModule(device_->logical(), &vertShaderModuleCreateInfo, core_->allocator(), &vertShaderModule) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateShaderModule failed");

          std::vector<uint32_t>    fragShaderCode = formattedSPIRV("./shaders/shader.frag.spv");
          VkShaderModuleCreateInfo fragShaderModuleCreateInfo {
               .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
               .pNext    = nullptr,
               .flags    = {},
               .codeSize = static_cast<uint32_t>(fragShaderCode.size()) * 4,
               .pCode    = fragShaderCode.data()
          };
          VkShaderModule fragShaderModule {};
          if (vkCreateShaderModule(device_->logical(), &fragShaderModuleCreateInfo, core_->allocator(), &fragShaderModule) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateShaderModule failed");

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
          auto extent = swapchain_->extent();

          VkViewport viewport {
               .x        = 0.f,
               .y        = 0.f,
               .width    = static_cast<float>(extent.width),
               .height   = static_cast<float>(extent.height),
               .minDepth = 0.f,
               .maxDepth = 1.f
          };

          VkRect2D scissorRect {
               .offset = { 0, 0 },
               .extent = extent
          };
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
               .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
               .primitiveRestartEnable = VK_FALSE
          };
          VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo {
               .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
               .pNext         = nullptr,
               .flags         = {},
               .viewportCount = 1,
               .pViewports    = &viewport,
               .scissorCount  = 1,
               .pScissors     = &scissorRect
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
               .rasterizationSamples  = VK_SAMPLE_COUNT_4_BIT,
               .sampleShadingEnable   = VK_TRUE,
               .minSampleShading      = 0.5f,
               .pSampleMask           = nullptr,
               .alphaToCoverageEnable = VK_FALSE,
               .alphaToOneEnable      = VK_FALSE
          };
          VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState {
               .blendEnable         = VK_FALSE,
               .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
               .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
               .colorBlendOp        = VK_BLEND_OP_ADD,
               .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
               .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
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
               .pDynamicState       = nullptr,
               .layout              = pipelineLayout_,
               .renderPass          = renderPass_->get(),
               .subpass             = 0u,
               .basePipelineHandle  = {},
               .basePipelineIndex   = -1
          };
          if (vkCreateGraphicsPipelines(device_->logical(), {}, 1u, &vkGraphicsPipelineCreateInfo, core_->allocator(), &pipeline_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateGraphicsPipelines failed");

          vkDestroyShaderModule(device_->logical(), vertShaderModule, core_->allocator());
          vkDestroyShaderModule(device_->logical(), fragShaderModule, core_->allocator());
     }
     ~GraphicsPipeline() {
          vkDestroyPipeline(device_->logical(), pipeline_, core_->allocator());
          vkDestroyPipelineLayout(device_->logical(), pipelineLayout_, core_->allocator());
     }
     
     auto& pipeline() { return pipeline_; }
     auto& layout() { return pipelineLayout_; }
     
  private:
     static std::vector<uint32_t> formattedSPIRV(std::string file_name) {
          std::basic_ifstream<char> file(file_name, std::ios::ate | std::ios::binary);
          if (!file.is_open())
               throw std::runtime_error("failed to open file \"" + file_name + "\"");

          const size_t fpos = static_cast<size_t>(file.tellg());

          std::vector<uint32_t> buffer(fpos / sizeof(uint32_t));

          file.seekg(0);
          file.read(reinterpret_cast<char*>(buffer.data()), fpos);
          file.close();

          return buffer;
     }

     Core*                core_;
     Device*              device_;
     Swapchain*           swapchain_;
     RenderPass*          renderPass_;
     DescriptorSetLayout* descriptorSetLayout_;
     VkPipelineLayout     pipelineLayout_;
     VkPipeline           pipeline_;
};
