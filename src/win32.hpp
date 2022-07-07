#pragma once

#include "event_dispatcher.hpp"

#include <windows.h>

#include <stdexcept>

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
     window make_window(const wchar_t* name, event_hub* dispatch);
     void process_messages();
     
  private:
     static LRESULT CALLBACK message_handler(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam);
     HINSTANCE hInstance_ {};
};
