#pragma once

#include "Buffer.hpp"
#include "Core.hpp"
// #include "Data.hpp"
// #include "DescriptorSets.hpp"
#include "Device.hpp"
#include "GraphicsPipeline.hpp"
#include "RenderPass.hpp"
#include "Vertex.hpp"

#include <chrono>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

// std::vector<Vertex> vertecies {
//      { .position           = { 0.f, -.5f, 0.f },
//         .color             = { 1.f, 1.f, 1.f },
//         .textureCoordinate = { 0.f, 0.f } },
//      { .position           = { -.5f, .5f, 0.f },
//         .color             = { 1.f, 1.f, 1.f },
//         .textureCoordinate = { 0.f, 0.f } },
//      { .position           = { .5f, .5f, 0.f },
//         .color             = { 1.f, 1.f, 1.f },
//         .textureCoordinate = { 0.f, 0.f } },
//      { .position           = { -.5f, -.5f, .5f },
//         .color             = { .5f, .5f, .5f },
//         .textureCoordinate = { 0.f, 0.f } },
//      { .position           = { 0.f, .5f, .5f },
//         .color             = { .5f, .5f, .5f },
//         .textureCoordinate = { 0.f, 0.f } },
//      { .position           = { .5f, -.5f, .5f },
//         .color             = { .5f, .5f, .5f },
//         .textureCoordinate = { 0.f, 0.f } }
// };

// std::vector<uint32_t> indecies = { 0, 1, 2, 3, 4, 5 };

class Renderer {
     // std::chrono::_V2::steady_clock::time_point           start   {std::chrono::steady_clock::now()};
  public:
     Renderer(Core* core, Win32* win32, Frame<Win32>* frame)
        : core_(core)
        , surface_(core_, win32, frame)
        , device_(core_)
        , swapchain_(Swapchain::make(&surface_, &device_, core_))
        , renderPass_(&device_, core_, &swapchain_)
        , descriptorSetLayout_(core_, &device_)
        , graphicsPipeline_(core_, &device_, &renderPass_, &descriptorSetLayout_)
        , commandPool_(&device_, core_)
        //    , texture_(core_, &device_, &commandPool_)
        //    , indexBuffer_(Buffer<uint32_t>::makeIndex(core_, &device_, &commandPool_, indecies))
        //    , uniformBuffers_([](Core* core, Device* device, CommandPool* commandPool, size_t maxFramesInFlight) {
        //         std::vector<Buffer<UniformBufferObject>> buffer;
        //         buffer.reserve(maxFramesInFlight);
        //         for (size_t i = 0; i != maxFramesInFlight; ++i) {
        //              buffer.push_back(Buffer<UniformBufferObject>::makeUniform(core, device, commandPool));
        //         }
        //         return buffer;
        //    }(core, &device_, &commandPool_, maxFramesInFlight_))
        //    , descriptorPool_(core_, &device_, &descriptorSetLayout_, maxFramesInFlight_, uniformBuffers_, &texture_)
        , renderCommandBuffers_(commandPool_.createCommandBuffers(swapchain_.imageCount()))
        , imageAvailable_(maxFramesInFlight_)
        , renderFinished_(maxFramesInFlight_)
        , imageInFlight_(maxFramesInFlight_) {
          auto extent = swapchain_.extent();
          width_ = extent.width;
          height_ = extent.height;
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
          width_ = x;
          height_ = y;
          if (width_ * height_ == 0)
               return;
          vkDeviceWaitIdle(device_.logical());
          swapchain_.update(&surface_);
          renderPass_.updateFramebuffers();
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

          // auto                  current = std::chrono::steady_clock::now();
          // auto                  delta   = static_cast<float>(std::sin(((current - start).count() % 6'283'185'307) / 1'000'000'000.f));
          // UniformBufferObject uniformBufferObject {
          //      .model = glm::rotate(glm::mat4(1.f), delta * glm::radians(45.f), glm::vec3(0.f, 0.f, 1.f)),
          //      .view  = glm::lookAt(glm::vec3(2.f, 2.f, 2.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f)),
          //      .proj  = glm::perspective(glm::radians(45.f), static_cast<float>(swapchain_.extent().width) / static_cast<float>(swapchain_.extent().height), 0.1f, 10.f)
          // };
          // uniformBufferObject.proj[1][1] *= -1.f;
          // uniformBuffers_[currentFrame_].write(&uniformBufferObject);

          if (vkResetFences(device_.logical(), 1, &imageInFlight_[currentFrame_]) != VK_SUCCESS)
               throw std::runtime_error("call to vkResetFences failed");
          vkResetCommandBuffer(renderCommandBuffers_[currentFrame_].get(), {});

          renderCommandBuffers_[currentFrame_].begin();
          {
               renderPass_.begin(swapchainImageIndex, &renderCommandBuffers_[currentFrame_]);
               {
                    vkCmdBindPipeline(renderCommandBuffers_[currentFrame_].get(), VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_.pipeline());

                    // vkCmdBindIndexBuffer(renderCommandBuffers_[currentFrame_].get(), indexBuffer_.get(), 0, VK_INDEX_TYPE_UINT32);
                    // vkCmdBindDescriptorSets(renderCommandBuffers_[currentFrame_].get(), VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_.layout(), 0, 1, &descriptorPool_.descriptorSet(currentFrame_), 0, nullptr);

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

                    for( auto& vertexBuffer : vertexBuffers_) {
                         VkBuffer     vertexBuffers[]     = { vertexBuffer.get() };
                         VkDeviceSize deviceSizeOffsets[] = { 0 };
                         vkCmdBindVertexBuffers(renderCommandBuffers_[currentFrame_].get(), 0, 1, vertexBuffers, deviceSizeOffsets);
                         // vkCmdDrawIndexed(renderCommandBuffers_[currentFrame_].get(), static_cast<uint32_t>(indecies.size()), 1, 0, 0, 0);
                         vkCmdDraw(renderCommandBuffers_[currentFrame_].get(), 4, 1, 0, 0);
                    }
               }
               renderPass_.end(&renderCommandBuffers_[currentFrame_]);
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
     Surface             surface_;
     Device              device_;
     Swapchain           swapchain_;
     RenderPass          renderPass_;
     DescriptorSetLayout descriptorSetLayout_;
     GraphicsPipeline    graphicsPipeline_;
     CommandPool         commandPool_;
     // Texture             texture_;

     std::vector<Buffer<Vertex>> vertexBuffers_;
     // Buffer<uint32_t> indexBuffer_;
     // std::vector<Buffer<UniformBufferObject>> uniformBuffers_;

     // DescriptorPool descriptorPool_;

     std::vector<CommandBuffer> renderCommandBuffers_;

     std::vector<VkSemaphore> imageAvailable_;
     std::vector<VkSemaphore> renderFinished_;
     std::vector<VkFence>     imageInFlight_;
     
     int width_;
     int height_;
     size_t     currentFrame_ { 0 };
     std::mutex swapchainMutex_;
};
