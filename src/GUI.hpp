#pragma once

#include "Renderer.hpp"
#include "Win32.hpp"

#include <fmt/xchar.h>

#include <chrono>
#include <thread>
#include <memory>
#include <string>
#include <vector>

class Window {
  public:
     Window(const wchar_t* name, Win32* win32, Core* core)
        : windowName_(name)
        , eventSystem_()
        , frame_(win32->createFrame(name, &eventSystem_))
        , renderer_(core, win32, &frame_) {
          onClose_     = subscribeOnClose([this] { shouldClose_ = true; });
          //onMouseMove_ = eventSystem_.mouseMoveDispatcher.subscribe([this](int x, int y) { fmt::print(L"Mouse moved to: {}, {} in window {}\n", x, y, windowName_); });
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

     std::unique_ptr<EventDispatcher<std::function<void()>>::Subscription>                   onClose_;
     //std::unique_ptr<EventDispatcher<std::function<void(int, int)>, int, int>::Subscription> onMouseMove_;
     
     Frame<Win32> frame_;
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
          loadModel();
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
