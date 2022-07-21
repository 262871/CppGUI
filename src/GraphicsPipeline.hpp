#pragma once

#include "CommandPool.hpp"
#include "Core.hpp"
#include "Vertex.hpp"
#include "DescriptorSets.hpp"
#include "Device.hpp"
#include "RenderPass.hpp"
#include "Swapchain.hpp"

#include <fstream>
#include <tuple>

class GraphicsPipeline {
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

  public:
     GraphicsPipeline(Core* core, Device* device, RenderPass* renderPass , DescriptorSetLayout* descriptorSetLayout)
        : core_(core)
        , device_(device)
        , renderPass_(renderPass)
     , descriptorSetLayout_(descriptorSetLayout) {
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
               .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
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

          VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo {
               .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
               .pNext = nullptr,
               .flags = {},
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
               .pDynamicState       = &pipelineDynamicStateCreateInfo,
               .layout              = pipelineLayout_,
               .renderPass          = renderPass_->get(),
               .subpass             = 0u,
               .basePipelineHandle  = {},
               .basePipelineIndex   = 0
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
     Core*       core_;
     Device*     device_;
     RenderPass* renderPass_;
     DescriptorSetLayout* descriptorSetLayout_;
     VkPipelineLayout pipelineLayout_;
     VkPipeline       pipeline_;
};
