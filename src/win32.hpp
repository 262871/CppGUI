#pragma once

#include <windows.h>

#include <stdexcept>

struct event_dispatcher {
     bool should_quit {false};
};

class win32 {
  public:
     class window {
       public:
          window(HWND hWnd);
          ~window();

       private:
          HWND hWnd_ {};
     };
     
     win32();
     window make_window(const wchar_t* name, event_dispatcher* dispatch);

  private:
     static LRESULT CALLBACK message_handler(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam);
     HINSTANCE hInstance_ {};
};
