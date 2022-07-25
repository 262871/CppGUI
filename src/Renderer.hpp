#pragma once

#include "Buffer.hpp"
#include "Core.hpp"
#include "RenderProgram.hpp"
// #include "Data.hpp"
// #include "DescriptorSets.hpp"
#include "Device.hpp"
#include "GraphicsPipeline.hpp"
#include "RenderPass.hpp"
#include "Vertex.hpp"

#include <chrono>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

class Renderer {
     static auto iConf(VkExtent2D extent) {
          return ImageResource2D::ImageConf {
               .format           = VK_FORMAT_B8G8R8A8_SRGB,
               .extent           = extent,
               .mipLevels        = 1,
               .msaa             = VK_SAMPLE_COUNT_4_BIT,
               .tiling           = VK_IMAGE_TILING_OPTIMAL,
               .usage            = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
               .memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
          };
     }
     static auto dConf(VkExtent2D extent) {
          return ImageResource2D::ImageConf {
               .format           = VK_FORMAT_D32_SFLOAT,
               .extent           = extent,
               .mipLevels        = 1,
               .msaa             = VK_SAMPLE_COUNT_4_BIT,
               .tiling           = VK_IMAGE_TILING_OPTIMAL,
               .usage            = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
               .memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
          };
     }

     // std::chrono::_V2::steady_clock::time_point           start   {std::chrono::steady_clock::now()};
  public:
     Renderer(Core* core, Surface* surface)
        : core_(core)
        , device_(core_)
        , commandPool_(core_, &device_)
        , renderCommandBuffers_(commandPool_.createCommandBuffers(2))
        , descriptorSetLayout_(core_, &device_)
        , descriptorPool_(core_, &device_, &descriptorSetLayout_)
        , swapchain_(core_, surface, &device_)
        , colorbuffer_(core_, &device_, iConf(swapchain_.extent()), { .aspect = VK_IMAGE_ASPECT_COLOR_BIT }) // could be better
        , depthbuffer_(core_, &device_, dConf(swapchain_.extent()), { .aspect = VK_IMAGE_ASPECT_DEPTH_BIT })
        , texture_(core_, &device_, &commandPool_)
        , renderProgram_(core_, &device_, &descriptorSetLayout_, { &swapchain_, &colorbuffer_, &depthbuffer_ }) // Good
     {
          descriptorSets_ = descriptorPool_.createDescriptorSets(maxFramesInFlight_, &texture_);

          auto extent = swapchain_.extent();
          width_      = extent.width;
          height_     = extent.height;

          VkSemaphoreCreateInfo vkSemaphoreCreateInfo {
               .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
               .pNext = nullptr,
               .flags = {}
          };
          VkFenceCreateInfo vkFenceCreateInfo {
               .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
               .pNext = nullptr,
               .flags = VK_FENCE_CREATE_SIGNALED_BIT
          };
          for (size_t i = 0; i != maxFramesInFlight_; ++i) {
               if (vkCreateSemaphore(device_.logical(), &vkSemaphoreCreateInfo, core_->allocator(), &imageAvailable_[i]) != VK_SUCCESS)
                    throw std::runtime_error("call to vkCreateSemaphore failed");
               if (vkCreateSemaphore(device_.logical(), &vkSemaphoreCreateInfo, core_->allocator(), &renderFinished_[i]) != VK_SUCCESS)
                    throw std::runtime_error("call to vkCreateSemaphore failed");
               if (vkCreateFence(device_.logical(), &vkFenceCreateInfo, core_->allocator(), &imageInFlight_[i]) != VK_SUCCESS)
                    throw std::runtime_error("call to vkCreateFence failed");
          }
     }

     ~Renderer() {
          vkDeviceWaitIdle(device_.logical());
          for (size_t i = 0; i != maxFramesInFlight_; ++i) {
               vkDestroySemaphore(device_.logical(), imageAvailable_[i], core_->allocator());
               vkDestroySemaphore(device_.logical(), renderFinished_[i], core_->allocator());
               vkDestroyFence(device_.logical(), imageInFlight_[i], core_->allocator());
          }
     }

     void load(std::vector<Vertex> vertecies) {
          vertexBuffers_.emplace_back(Buffer<Vertex>::makeVertex(core_, &device_, &commandPool_, vertecies));
     }

     void resize(int x, int y) {
          std::unique_lock<std::mutex> swapchain_lock(swapchainMutex_);
          width_  = x;
          height_ = y;
          if (width_ * height_ == 0)
               return;
          vkDeviceWaitIdle(device_.logical());
          swapchain_.resize();
          colorbuffer_.resize(iConf(swapchain_.extent()), { .aspect = VK_IMAGE_ASPECT_COLOR_BIT });
          depthbuffer_.resize(dConf(swapchain_.extent()), { .aspect = VK_IMAGE_ASPECT_DEPTH_BIT });
          renderProgram_.resize();
     }

     int width() { return width_; }
     int height() { return height_; }

     void tryDrawFrame() {
          std::unique_lock<std::mutex> swapchain_lock(swapchainMutex_);
          if (width_ * height_ == 0)
               return;

          if (vkWaitForFences(device_.logical(), 1, &imageInFlight_[currentFrame_], VK_FALSE, 4000000000) != VK_SUCCESS)
               throw std::runtime_error("failed to wait for in flight fence");

          uint32_t swapchainImageIndex;
          if (vkAcquireNextImageKHR(device_.logical(), swapchain_.get(), 4000000000, imageAvailable_[currentFrame_], VK_NULL_HANDLE, &swapchainImageIndex) != VK_SUCCESS)
               return;

          if (vkResetFences(device_.logical(), 1, &imageInFlight_[currentFrame_]) != VK_SUCCESS)
               throw std::runtime_error("call to vkResetFences failed");
          vkResetCommandBuffer(renderCommandBuffers_[currentFrame_].get(), {});

          renderCommandBuffers_[currentFrame_].begin();
          {
               renderProgram_.beginRenderPass(swapchainImageIndex, &renderCommandBuffers_[currentFrame_]);
               {
                    vkCmdBindPipeline(renderCommandBuffers_[currentFrame_].get(), VK_PIPELINE_BIND_POINT_GRAPHICS, renderProgram_.pipeline());
                    VkViewport viewport {
                         .x        = 0.f,
                         .y        = 0.f,
                         .width    = static_cast<float>(swapchain_.extent().width),
                         .height   = static_cast<float>(swapchain_.extent().height),
                         .minDepth = 0.f,
                         .maxDepth = 1.f
                    };
                    VkRect2D scissor {
                         .offset = { 0, 0 },
                         .extent = swapchain_.extent()
                    };
                    vkCmdSetViewport(renderCommandBuffers_[currentFrame_].get(), 0, 1, &viewport);
                    vkCmdSetScissor(renderCommandBuffers_[currentFrame_].get(), 0, 1, &scissor);

                    for (auto& vertexBuffer : vertexBuffers_) {
                         VkBuffer     vertexBuffers[]     = { vertexBuffer.get() };
                         VkDeviceSize deviceSizeOffsets[] = { 0 };
                         vkCmdBindVertexBuffers(renderCommandBuffers_[currentFrame_].get(), 0, 1, vertexBuffers, deviceSizeOffsets);
                         vkCmdBindDescriptorSets(renderCommandBuffers_[currentFrame_].get(), VK_PIPELINE_BIND_POINT_GRAPHICS, renderProgram_.pipelineLayout(), 0, 1, &descriptorSets_[currentFrame_], 0, nullptr);
                         vkCmdDraw(renderCommandBuffers_[currentFrame_].get(), 4, 1, 0, 0);
                    }
               }
               renderProgram_.endRenderPass(&renderCommandBuffers_[currentFrame_]);
          }
          renderCommandBuffers_[currentFrame_].end();

          VkPipelineStageFlags pipeline_stage_flags { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

          VkSubmitInfo submit_info {
               .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
               .pNext                = nullptr,
               .waitSemaphoreCount   = 1,
               .pWaitSemaphores      = &imageAvailable_[currentFrame_],
               .pWaitDstStageMask    = &pipeline_stage_flags,
               .commandBufferCount   = 1,
               .pCommandBuffers      = &renderCommandBuffers_[currentFrame_].get(),
               .signalSemaphoreCount = 1,
               .pSignalSemaphores    = &renderFinished_[currentFrame_]
          };

          if (vkQueueSubmit(device_.graphics(), 1, &submit_info, imageInFlight_[currentFrame_]) != VK_SUCCESS)
               throw std::runtime_error("call to vkQueueSubmit failed");

          VkPresentInfoKHR present_info {
               .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
               .pNext              = nullptr,
               .waitSemaphoreCount = 1,
               .pWaitSemaphores    = &renderFinished_[currentFrame_],
               .swapchainCount     = 1,
               .pSwapchains        = &swapchain_.get(),
               .pImageIndices      = &swapchainImageIndex,
               .pResults           = nullptr
          };
          vkQueuePresentKHR(device_.present(), &present_info);

          currentFrame_ = (1 + currentFrame_) % maxFramesInFlight_;
     }

  private:
     const size_t        maxFramesInFlight_ { 2 };
     Core*               core_;
     Device              device_;
     CommandPool         commandPool_;
     std::vector<CommandBuffer> renderCommandBuffers_;
     DescriptorSetLayout descriptorSetLayout_;
     DescriptorPool      descriptorPool_;

     Swapchain       swapchain_;
     ImageResource2D colorbuffer_;
     ImageResource2D depthbuffer_;

     Texture        texture_;
     RenderProgram renderProgram_;

     std::vector<VkDescriptorSet> descriptorSets_;
     std::vector<Buffer<Vertex>>  vertexBuffers_;


     std::vector<VkSemaphore> imageAvailable_ { 2 };
     std::vector<VkSemaphore> renderFinished_ { 2 };
     std::vector<VkFence>     imageInFlight_ { 2 };

     int        width_;
     int        height_;
     size_t     currentFrame_ { 0 };
     std::mutex swapchainMutex_;
};
