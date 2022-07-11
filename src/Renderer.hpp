#pragma once

#include "Core.hpp"
#include "DebugMessenger.hpp"
#include "Device.hpp"
#include "RenderPass.hpp"

class Renderer {
  public:
     Renderer(Core* core, Win32* win32, Frame<Win32>* frame)
        : surface_(core, win32, frame)
        , device_(core)
        , swapchain_(Swapchain::make(&surface_, &device_, core))
        , renderPass_(&device_, core, &swapchain_)
        , graphicsCommandPool_(&device_, core, 0) {}

  private:
     Surface     surface_;
     Device      device_;
     Swapchain   swapchain_;
     RenderPass  renderPass_;
     CommandPool graphicsCommandPool_;
};
