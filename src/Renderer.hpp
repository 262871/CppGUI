#pragma once

#include "Core.hpp"
#include "DebugMessenger.hpp"
#include "Device.hpp"
#include "RenderPass.hpp"

class Renderer {
  public:
     Renderer(Core* core, Win32* win32, Frame<Win32>* frame)
        : core_(core)
        , surface_(core_, win32, frame)
        , device_(core_)
        , swapchain_(Swapchain::make(&surface_, &device_, core_))
        , renderPass_(&device_, core_, &swapchain_)
        , graphicsCommandPool_(&device_, core_, 0)
        , renderCommandBuffers_(graphicsCommandPool_.createCommandBuffers(swapchain_.imageCount()))
        , imageAvailable_(maxFramesInFlight_)
        , renderFinished_(maxFramesInFlight_)
        , imageInFlight_(maxFramesInFlight_) {
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
          for (size_t i = 0; i != maxFramesInFlight_; ++i) {
               vkDestroySemaphore(device_.logical(), imageAvailable_[i], core_->allocator());
               vkDestroySemaphore(device_.logical(), renderFinished_[i], core_->allocator());
               vkDestroyFence(device_.logical(), imageInFlight_[i], core_->allocator());
          }
     }

     void tryDrawFrame() {
          std::unique_lock<std::mutex> swapchain_lock(swapchainMutex_);
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
               renderPass_.begin(swapchainImageIndex, &renderCommandBuffers_[currentFrame_]);
               {
                    
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
     Core*       core_;
     Surface     surface_;
     Device      device_;
     Swapchain   swapchain_;
     RenderPass  renderPass_;
     CommandPool graphicsCommandPool_;

     std::vector<CommandBuffer> renderCommandBuffers_;

     size_t                   currentFrame_ { 0 };
     const size_t             maxFramesInFlight_ { 2 };
     std::vector<VkSemaphore> imageAvailable_;
     std::vector<VkSemaphore> renderFinished_;
     std::vector<VkFence>     imageInFlight_;

     std::mutex swapchainMutex_;
};
