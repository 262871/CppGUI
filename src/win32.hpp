#pragma once

#include "EventSystem.hpp"

#include <windows.h>
#include <windowsx.h>

#include <stdexcept>

class Win32 {
  public:
     Win32() { hInstance_ = GetModuleHandleW(0); }
     HINSTANCE instance() const { return hInstance_; }

     void processMessages();

     using frame_handle = HWND;

     frame_handle createFrame(const wchar_t* name, EventSystem* eventSystem);
     static void  destroyFrame(frame_handle hWnd) { DestroyWindow(hWnd); }

  private:
     static LRESULT CALLBACK messageHandler(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam);

     HINSTANCE hInstance_;
};

void Win32::processMessages() {
     MSG message;
     while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {
          TranslateMessage(&message);
          DispatchMessageW(&message);
     }
}

Win32::frame_handle Win32::createFrame(const wchar_t* name, EventSystem* eventSystem) {
     WNDCLASSEXW wcx {
          .cbSize        = sizeof(WNDCLASSEX),
          .style         = CS_DBLCLKS,
          .lpfnWndProc   = messageHandler,
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
     auto lpParam = reinterpret_cast<LPVOID>(eventSystem);
     auto hWnd    = CreateWindowExW(WS_EX_APPWINDOW, name, name, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance_, lpParam);
     if (!hWnd)
          throw std::runtime_error("call to CreateWindowExW failed");
     ShowWindow(hWnd, SW_SHOWNOACTIVATE);
     return hWnd;
}

LRESULT CALLBACK Win32::messageHandler(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam) {
     EventSystem* eventSystem { nullptr };
     if (uMsg == WM_CREATE) {
          auto pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
          eventSystem  = reinterpret_cast<EventSystem*>(pCreate->lpCreateParams);
          SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreate->lpCreateParams));
     }
     else
          eventSystem = reinterpret_cast<EventSystem*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

     switch (uMsg) {
          case WM_ERASEBKGND:
               return 1;
          case WM_CLOSE:
               eventSystem->closeDispatcher.signal();
               return 0;
          case WM_DESTROY:
               PostQuitMessage(0);
               return 0;
          case WM_SIZE: {
               RECT r;
               GetClientRect(hWnd, &r);
               eventSystem->resizeDispatcher.signal(r.right - r.left, r.bottom - r.top);
          } break;
          case WM_KEYDOWN:
          case WM_SYSKEYDOWN:
          case WM_KEYUP:
          case WM_SYSKEYUP:
               // (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
               // (HIWORD(lParam) & KF_EXTENDED) == KF_EXTENDED;
               return 0;

          case WM_MOUSEMOVE:
               eventSystem->mouseMoveDispatcher.signal(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
               break;
          case WM_MOUSEWHEEL:
               break;
          case WM_LBUTTONDOWN:
          case WM_MBUTTONDOWN:
          case WM_RBUTTONDOWN:
          case WM_LBUTTONUP:
          case WM_MBUTTONUP:
          case WM_RBUTTONUP:
               eventSystem->mouseButtonDispatcher.signal(EventSystem::MouseButton::LEFT, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
               // WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
               break;
     }
     return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
