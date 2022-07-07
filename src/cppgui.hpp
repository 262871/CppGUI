#pragma once

#include "debug_messenger.hpp"
#include "instance.hpp"
#include "win32.hpp"

class cppgui {
  public:
     cppgui();
     void run();

  private:
     using close_sub = event_dispatcher<std::function<void(void)>>::subscription;

     instance        instance_;
     debug_messenger messenger_;
     bool            should_close_;
     event_hub       user_;
     close_sub       close_listener_;
     win32           platform_;
     win32::window   window_;
};
