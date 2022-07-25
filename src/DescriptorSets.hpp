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

  public:
     DescriptorPool(Core* core, Device* device, DescriptorSetLayout* descriptorSetLayout)
        : core_(core)
        , device_(device)
        , descriptorSetLayout_(descriptorSetLayout) {
          std::vector<VkDescriptorPoolSize> descriptorPoolSizes {
               { .type             = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                  .descriptorCount = static_cast<uint32_t>(2) }
          };
          VkDescriptorPoolCreateInfo descriptorPoolCreateInfo {
               .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
               .pNext         = nullptr,
               .flags         = {},
               .maxSets       = static_cast<uint32_t>(2),
               .poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
               .pPoolSizes    = descriptorPoolSizes.data()
          };
          if (vkCreateDescriptorPool(device_->logical(), &descriptorPoolCreateInfo, core_->allocator(), &descriptorPool_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateDescriptorPool failed");
     }

     ~DescriptorPool() {
          vkDestroyDescriptorPool(device_->logical(), descriptorPool_, core_->allocator());
     }
     

     auto createDescriptorSets(size_t maxFramesInFlight, Texture* texture) {
          std::vector<VkDescriptorSet> descriptorSets(maxFramesInFlight);
          std::vector<VkDescriptorSetLayout> descriptorSetLayouts(maxFramesInFlight, descriptorSetLayout_->get());
          VkDescriptorSetAllocateInfo        descriptorSetAllocateInfo {
                      .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                      .pNext              = nullptr,
                      .descriptorPool     = descriptorPool_,
                      .descriptorSetCount = static_cast<uint32_t>(maxFramesInFlight),
                      .pSetLayouts        = descriptorSetLayouts.data()
          };
          
          if (vkAllocateDescriptorSets(device_->logical(), &descriptorSetAllocateInfo, descriptorSets.data()) != VK_SUCCESS)
               throw std::runtime_error("call to vkAllocateDescriptorSets failed");

          for (size_t i = 0; i != maxFramesInFlight; ++i) {
               VkDescriptorImageInfo descriptorImageInfo {
                    .sampler     = texture->sampler(),
                    .imageView   = texture->view(),
                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
               };
               std::vector<VkWriteDescriptorSet> writeDescriptorSet {
                    { .sType             = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                       .pNext            = nullptr,
                       .dstSet           = descriptorSets[i],
                       .dstBinding       = 0,
                       .dstArrayElement  = 0,
                       .descriptorCount  = 1,
                       .descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                       .pImageInfo       = &descriptorImageInfo,
                       .pBufferInfo      = nullptr,
                       .pTexelBufferView = nullptr }
               };
               vkUpdateDescriptorSets(device_->logical(), static_cast<uint32_t>(writeDescriptorSet.size()), writeDescriptorSet.data(), 0, nullptr);
          }
          return descriptorSets;
     }
  private:
};
