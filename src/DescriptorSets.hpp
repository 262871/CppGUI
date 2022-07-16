#pragma once

#include "Buffer.hpp"
#include "Core.hpp"
#include "Device.hpp"
#include "Texture.hpp"

class DescriptorSetLayout {
     Core*                 core_;
     Device*               device_;
     VkDescriptorSetLayout descriptorSetLayout_;

  public:
     DescriptorSetLayout(Core* core, Device* device)
        : core_(core)
        , device_(device) {
          std::vector<VkDescriptorSetLayoutBinding> bindings {
               { .binding             = 0,
                  .descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                  .descriptorCount    = 1,
                  .stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
                  .pImmutableSamplers = nullptr },
               { .binding             = 1,
                  .descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                  .descriptorCount    = 1,
                  .stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
                  .pImmutableSamplers = nullptr }
          };

          VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo {
               .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
               .pNext        = nullptr,
               .flags        = {},
               .bindingCount = static_cast<uint32_t>(bindings.size()),
               .pBindings    = bindings.data()
          };
          if (vkCreateDescriptorSetLayout(device_->logical(), &vkDescriptorSetLayoutCreateInfo, core_->allocator(), &descriptorSetLayout_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateDescriptorSetLayout failed");
     }
     ~DescriptorSetLayout() {
          vkDestroyDescriptorSetLayout(device_->logical(), descriptorSetLayout_, core_->allocator());
     }
     auto& get() { return descriptorSetLayout_; }
};

class DescriptorPool {
     Core*                        core_;
     Device*                      device_;
     DescriptorSetLayout*         descriptorSetLayout_;
     VkDescriptorPool             descriptorPool_;
     std::vector<VkDescriptorSet> descriptorSets_;

  public:
     DescriptorPool(Core* core, Device* device, DescriptorSetLayout* descriptorSetLayout, size_t maxFramesInFlight, std::vector<Buffer<UniformBufferObject>>& uniformBuffers, Texture* texture)
        : core_(core)
        , device_(device)
        , descriptorSetLayout_(descriptorSetLayout) {
          std::vector<VkDescriptorPoolSize> descriptorPoolSizes {
               { .type             = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                  .descriptorCount = static_cast<uint32_t>(maxFramesInFlight) },
               { .type             = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                  .descriptorCount = static_cast<uint32_t>(maxFramesInFlight) }
          };

          VkDescriptorPoolCreateInfo descriptorPoolCreateInfo {
               .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
               .pNext         = nullptr,
               .flags         = {},
               .maxSets       = static_cast<uint32_t>(maxFramesInFlight),
               .poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
               .pPoolSizes    = descriptorPoolSizes.data()
          };

          if (vkCreateDescriptorPool(device_->logical(), &descriptorPoolCreateInfo, core_->allocator(), &descriptorPool_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateDescriptorPool failed");

          createDescriptorSets(maxFramesInFlight, uniformBuffers, texture);
     }

     ~DescriptorPool() {
          vkDestroyDescriptorPool(device_->logical(), descriptorPool_, core_->allocator());
     }
     
     auto& descriptorSet(size_t index) { return descriptorSets_[index]; }

  private:
     void createDescriptorSets(size_t maxFramesInFlight, std::vector<Buffer<UniformBufferObject>>& uniformBuffers, Texture* texture) {
          std::vector<VkDescriptorSetLayout> descriptorSetLayouts(maxFramesInFlight, descriptorSetLayout_->get());
          VkDescriptorSetAllocateInfo        descriptorSetAllocateInfo {
                      .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                      .pNext              = nullptr,
                      .descriptorPool     = descriptorPool_,
                      .descriptorSetCount = static_cast<uint32_t>(maxFramesInFlight),
                      .pSetLayouts        = descriptorSetLayouts.data()
          };
          descriptorSets_.reserve(maxFramesInFlight);
          if (vkAllocateDescriptorSets(device_->logical(), &descriptorSetAllocateInfo, descriptorSets_.data()) != VK_SUCCESS)
               throw std::runtime_error("call to vkAllocateDescriptorSets failed");

          for (size_t i = 0; i != maxFramesInFlight; ++i) {
               VkDescriptorBufferInfo descriptorBufferInfo {
                    .buffer = uniformBuffers[i].get(),
                    .offset = 0,
                    .range  = sizeof(UniformBufferObject)
               };
               VkDescriptorImageInfo descriptorImageInfo {
                    .sampler     = texture->sampler(),
                    .imageView   = texture->view(),
                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
               };
               std::vector<VkWriteDescriptorSet> write_descriptor_sets {
                    { .sType             = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                       .pNext            = nullptr,
                       .dstSet           = descriptorSets_[i],
                       .dstBinding       = 0,
                       .dstArrayElement  = 0,
                       .descriptorCount  = 1,
                       .descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                       .pImageInfo       = nullptr,
                       .pBufferInfo      = &descriptorBufferInfo,
                       .pTexelBufferView = nullptr },
                    { .sType             = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                       .pNext            = nullptr,
                       .dstSet           = descriptorSets_[i],
                       .dstBinding       = 1,
                       .dstArrayElement  = 0,
                       .descriptorCount  = 1,
                       .descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                       .pImageInfo       = &descriptorImageInfo,
                       .pBufferInfo      = nullptr,
                       .pTexelBufferView = nullptr }
               };
               vkUpdateDescriptorSets(device_->logical(), static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
          }
     }
};
