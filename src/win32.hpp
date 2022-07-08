#pragma once

#include "event.hpp"

#include <stdexcept>

#include <windows.h>

class win32 {
  public:
     win32() { hInstance_ = GetModuleHandleW(0); }
     HINSTANCE instance() const { return hInstance_; }

     void process_messages();

     using frame_handle = HWND;

     frame_handle create_frame(const wchar_t* name, event_system* events);
     static void  destroy_frame(frame_handle hWnd) { DestroyWindow(hWnd); }

  private:
     static LRESULT CALLBACK message_handler(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam);

     HINSTANCE hInstance_;
};

void win32::process_messages() {
     MSG message;
     while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {
          TranslateMessage(&message);
          DispatchMessageW(&message);
     }
}

win32::frame_handle win32::create_frame(const wchar_t* name, event_system* events) {
     WNDCLASSEXW wcx {
          .cbSize        = sizeof(WNDCLASSEX),
          .style         = CS_DBLCLKS,
          .lpfnWndProc   = message_handler,
          .cbClsExtra    = 0,
          .cbWndExtra    = 0,
          .hInstance     = hInstance_,
          .hIcon         = LoadIcon(hInstance_, IDI_APPLICATION),
          .hCursor       = LoadCursor(NULL, IDC_ARROW),
          .hbrBackground = NULL,
          .lpszMenuName  = NULL,
          .lpszClassName = name,
          .hIconSm       = NULL
     };
     if (!RegisterClassExW(&wcx))
          throw std::runtime_error("call to RegisterClassExW failed");
     auto lpParam = reinterpret_cast<LPVOID>(events);
     auto hWnd    = CreateWindowExW(WS_EX_APPWINDOW, name, name, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance_, lpParam);
     if (!hWnd)
          throw std::runtime_error("call to CreateWindowExW failed");
     ShowWindow(hWnd, SW_SHOWNOACTIVATE);
     return hWnd;
}

LRESULT CALLBACK win32::message_handler(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam) {
     event_system* owner { nullptr };
     if (uMsg == WM_CREATE) {
          auto pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
          owner        = reinterpret_cast<event_system*>(pCreate->lpCreateParams);
          SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreate->lpCreateParams));
     }
     else
          owner = reinterpret_cast<event_system*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

     switch (uMsg) {
          case WM_ERASEBKGND:
               return 1;
          case WM_CLOSE:
               owner->should_close = true;
               return 0;
          case WM_DESTROY:
               PostQuitMessage(0);
               return 0;
          case WM_SIZE:
               // RECT r;
               // GetClientRect(hWnd, &r);
               break;
          case WM_KEYDOWN:
          case WM_SYSKEYDOWN:
          case WM_KEYUP:
          case WM_SYSKEYUP:
               // (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
               // (HIWORD(lParam) & KF_EXTENDED) == KF_EXTENDED;
               return 0;

          case WM_MOUSEMOVE:
               // GET_X_LPARAM(lParam);
               // GET_Y_LPARAM(lParam);
               break;
          case WM_MOUSEWHEEL:
               break;
          case WM_LBUTTONDOWN:
          case WM_MBUTTONDOWN:
          case WM_RBUTTONDOWN:
          case WM_LBUTTONUP:
          case WM_MBUTTONUP:
          case WM_RBUTTONUP:
               // WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
               break;
     }
     return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
