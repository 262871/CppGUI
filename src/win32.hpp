#pragma once

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include <windows.h>

#include <stdexcept>

template <typename T>
concept event_handling = requires(T t) {
     t.should_quit;
};

class win32 {
     template <typename T>
     requires event_handling<T>
     static LRESULT CALLBACK message_handler(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam) {
          T* window_user { nullptr };
          if (uMsg == WM_CREATE) {
               auto pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
               window_user  = reinterpret_cast<T*>(pCreate->lpCreateParams);
               SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreate->lpCreateParams));
          }
          else
               window_user = reinterpret_cast<T*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));

          switch (uMsg) {
               case WM_ERASEBKGND:
                    return 1;
               case WM_CLOSE:
                    window_user->should_quit = true;
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

  public:
     using window_handle_type = HWND;

     static win32& get_instance_ref() {
          static win32 instance;
          return instance;
     }
     template <typename T>
     window_handle_type make_window(const wchar_t* name, T* pApp) {
          WNDCLASSEXW wcx {
               .cbSize        = sizeof(WNDCLASSEX),
               .style         = CS_DBLCLKS,
               .lpfnWndProc   = message_handler<T>,
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
          auto pUser = reinterpret_cast<LPVOID>(pApp);
          HWND hWnd  = CreateWindowExW(WS_EX_APPWINDOW, name, name, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance_, pUser);
          if (!hWnd)
               throw std::runtime_error("call to CreateWindowExW failed");

          ShowWindow(hWnd, SW_SHOWNOACTIVATE);
          return hWnd;
     }
     void destroy_window(window_handle_type handle) {
          DestroyWindow(handle);
     }

  private:
     win32()
        : hInstance_(GetModuleHandleW(0)) {
     }
     ~win32() {
     }
     win32(const win32&)            = delete;
     win32(win32&&)                 = delete;
     win32& operator=(const win32&) = delete;
     win32& operator=(win32&&)      = delete;

     HINSTANCE hInstance_ {};
};
