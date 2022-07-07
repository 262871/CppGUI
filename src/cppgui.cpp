#include "cppgui.hpp"

cppgui::cppgui()
   : instance_(instance())
   , messenger_(debug_messenger(instance_.get()))
   , should_close_(false)
   , user_(event_hub())
   , close_listener_(user_.close_dispatcher.subscribe([this] { should_close_ = true; }))
   , platform_(win32())
   , window_(platform_.make_window(L"Vulkan win32", &user_)) {
}

void cppgui::run() {
     while (!should_close_) {
          platform_.process_messages();
     }
}
