#include "cppgui.hpp"

cppgui::cppgui()
   : should_close_(false)
   , events_(event_hub())
   , close_listener_(events_.close_dispatcher.subscribe([this] { should_close_ = true; }))
   , platform_(win32())
   , window_(platform_.make_window(L"Vulkan win32", &events_))
   , engine_(&platform_, &window_) {
}

void cppgui::run() {
     while (!should_close_) {
          platform_.process_messages();
     }
}
