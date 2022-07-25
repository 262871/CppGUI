#pragma once

#include "DebugMessenger.hpp"
#include "Renderer.hpp"
#include "Win32.hpp"

#include <fmt/xchar.h>

#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>

class Window {
  public:
     Window(const wchar_t* name, Win32* win32, Core* core)
        : windowName_(name)
        , eventSystem_()
        , frame_(win32->createFrame(name, &eventSystem_))
        , surface_(core, win32, &frame_)
        , renderer_(core, &surface_) {
          // std::vector<Vertex> vertecies {
          //      { .position           = { -.5f, -.5f, 0.1f },
          //         .color             = { 1.f, 1.f, 1.f },
          //         .textureCoordinate = { 0.f, 0.f } },
          //      { .position           = { -.5f, .5f, 0.1f },
          //         .color             = { 1.f, 1.f, 1.f },
          //         .textureCoordinate = { 0.f, 1.f } },
          //      { .position           = { .5f, -.5f, 0.1f },
          //         .color             = { 1.f, 1.f, 1.f },
          //         .textureCoordinate = { 1.f, 0.f } },
          //      { .position           = { .5f, .5f, 0.1f },
          //         .color             = { .5f, .5f, .5f },
          //         .textureCoordinate = { 1.f, 1.f } }
          // };
          // renderer_.load(vertecies);
          onClose_  = subscribeOnClose([this] { shouldClose_ = true; });
          onResize_ = eventSystem_.resizeDispatcher.subscribe([this](int x, int y) {
               renderer_.resize(x, y);
               renderer_.tryDrawFrame();
          });
          onClick_  = eventSystem_.mouseButtonDispatcher.subscribe([this](EventSystem::MouseButton button, int x, int y) {
               auto w = (2.f * static_cast<float>(x) / static_cast<float>(renderer_.width())) - 1.f;
               auto h = (2.f * static_cast<float>(y) / static_cast<float>(renderer_.height())) - 1.f;
               if (button == EventSystem::MouseButton::LEFT) {
                    std::vector<Vertex> verts {
                         { .position           = { w - .1f, h - .1f, 0.f },
                             .color             = { 1.f, 1.f, 1.f },
                             .textureCoordinate = { 0.f, 0.f } },
                         { .position           = { w - .1f, h + .1f, 0.f },
                             .color             = { .9f, .9f, .9f },
                             .textureCoordinate = { 0.f, 1.f } },
                         { .position           = { w + .1f, h - .1f, 0.f },
                             .color             = { .8f, .8f, .8f },
                             .textureCoordinate = { 1.f, 0.f } },
                         { .position           = { w + .1f, h + .1f, 0.f },
                             .color             = { .5f, .5f, .5f },
                             .textureCoordinate = { 1.f, 1.f } }
                    };
                    renderer_.load(verts);
               }
          });
          // onMouseMove_ = eventSystem_.mouseMoveDispatcher.subscribe([this](int x, int y) { fmt::print(L"Mouse moved to: {}, {} in window {}\n", x, y, windowName_); });
     }
     Window(const Window&)            = delete;
     Window(Window&&)                 = delete;
     Window& operator=(const Window&) = delete;
     Window& operator=(Window&&)      = delete;

     std::unique_ptr<EventDispatcher<std::function<void()>>::Subscription> subscribeOnClose(std::function<void()> callback) {
          return eventSystem_.closeDispatcher.subscribe(callback);
     }

     bool shouldClose() const { return shouldClose_; }

     std::wstring windowName_;
     EventSystem  eventSystem_;
     bool         shouldClose_ { false };

     std::unique_ptr<EventDispatcher<std::function<void()>>::Subscription>                                                                       onClose_;
     std::unique_ptr<EventDispatcher<std::function<void(EventSystem::MouseButton, int, int)>, EventSystem::MouseButton, int, int>::Subscription> onClick_;
     std::unique_ptr<EventDispatcher<std::function<void(int, int)>, int, int>::Subscription>                                                     onResize_;
     // std::unique_ptr<EventDispatcher<std::function<void(int, int)>, int, int>::Subscription> onMouseMove_;

     Frame<Win32> frame_;
     Surface surface_;
     Renderer     renderer_;

  private:
};

class GUI {
  public:
     GUI()
        : win32_()
        , core_()
        , debug_(&core_) {}
     ~GUI() {
          shutdown();
     }
     void shutdown() {
     }
     void initialize() {
          // loadModel();
          auto name = L"Vulkan and win32";
          windows_.emplace_back(std::make_unique<Window>(name, &win32_, &core_));
          // name = L"Second window";
          // windows_.emplace_back(std::make_unique<Window>(name, &win32_, &core_));
     }
     void run() {
          initialize();
          while (!windows_.empty()) {
               win32_.processMessages();
               for (auto& window : windows_)
                    window->renderer_.tryDrawFrame();
               std::erase_if(windows_, [](const auto& w) { return w->shouldClose(); });
               std::this_thread::sleep_for(std::chrono::milliseconds(16));
          }
     }

  private:
     Win32          win32_;
     Core           core_;
     DebugMessenger debug_;

     std::vector<std::unique_ptr<Window>> windows_ {};
};
