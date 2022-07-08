#pragma once

#include "win32.hpp"
#include "engine.hpp"

class cppgui {
  public:
     cppgui();
     void run();

  private:
     using void_sub = event_dispatcher<std::function<void(void)>>::subscription;

     bool          should_close_;
     event_hub     events_;
     void_sub      close_listener_;
     win32         platform_;
     win32::window window_;
     engine        engine_;
};
