#include <volk.h>

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include <windows.h>

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void*) {
     if (severity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
          fmt::print(stderr, "Error: {}\n", callback_data->pMessage);
     else if (severity != VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
          fmt::print(stdout, "Varning: {}\n", callback_data->pMessage);
     else
          fmt::print(stdout, "Info: {}\n", callback_data->pMessage);
     return VK_FALSE;
}
bool should_quit {false};
LRESULT CALLBACK message_handler(HWND hwnd, uint32_t msg, WPARAM w_param, LPARAM l_param) {
     switch (msg) {
          case WM_ERASEBKGND:
               return 1;
          case WM_CLOSE:
               should_quit = true;
               return 0;
          case WM_DESTROY:
               PostQuitMessage(0);
               return 0;
          case WM_SIZE:
               // RECT r;
               // GetClientRect(hwnd, &r);
               break;
          case WM_KEYDOWN:
          case WM_SYSKEYDOWN:
          case WM_KEYUP:
          case WM_SYSKEYUP:
               // (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
               // (HIWORD(l_param) & KF_EXTENDED) == KF_EXTENDED;
               return 0;

          case WM_MOUSEMOVE:
               // GET_X_LPARAM(l_param);
               // GET_Y_LPARAM(l_param);
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

     return DefWindowProcA(hwnd, msg, w_param, l_param);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
     HINSTANCE  h_instance = GetModuleHandleA(0);
     WNDCLASSEX wcx {
          .cbSize        = sizeof(WNDCLASSEX),
          .style         = CS_DBLCLKS,
          .lpfnWndProc   = message_handler,
          .cbClsExtra    = 0,
          .cbWndExtra    = 0,
          .hInstance     = h_instance,
          .hIcon         = LoadIcon(h_instance, IDI_APPLICATION),
          .hCursor       = LoadCursor(NULL, IDC_ARROW),
          .hbrBackground = NULL,
          .lpszMenuName  = NULL,
          .lpszClassName = "262871",
          .hIconSm       = NULL
     };
     if (!RegisterClassEx(&wcx))
          return 1;

     HWND hwnd = CreateWindowEx(WS_EX_APPWINDOW, "262871", "Vulkan on win32", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, h_instance, NULL);
     if (!hwnd)
          return 2;
     ShowWindow(hwnd, SW_SHOWNOACTIVATE);

     if (volkInitialize() != VK_SUCCESS)
          return 3;

     uint32_t extensions {};
     uint32_t version {};
     vkEnumerateInstanceExtensionProperties(nullptr, &extensions, nullptr);
     vkEnumerateInstanceVersion(&version);
     fmt::print("{} extensions supported\nVulkan version: {}.{}.{}\n", extensions, VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version), VK_VERSION_PATCH(version));

     VkApplicationInfo application_info {
          .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
          .pNext              = nullptr,
          .pApplicationName   = "GUI",
          .applicationVersion = 1,
          .pEngineName        = "CppGUI",
          .engineVersion      = 1,
          .apiVersion         = version
     };
     auto                 debug_ext { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };
     VkInstanceCreateInfo instance_create_info {
          .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
          .pNext                   = nullptr,
          .flags                   = {},
          .pApplicationInfo        = &application_info,
          .enabledLayerCount       = 0,
          .ppEnabledLayerNames     = nullptr,
          .enabledExtensionCount   = 1,
          .ppEnabledExtensionNames = &debug_ext
     };
     VkInstance instance {};
     if (vkCreateInstance(&instance_create_info, nullptr, &instance) != VK_SUCCESS)
          return 4;
     volkLoadInstance(instance);

     uint32_t device_count {};
     if (vkEnumeratePhysicalDevices(instance, &device_count, nullptr) != VK_SUCCESS)
          return 5;
     fmt::print("{} devices present\n", device_count);

     VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info {
          .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
          .pNext           = nullptr,
          .flags           = {},
          .messageSeverity = { VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT },
          .messageType     = { VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT },
          .pfnUserCallback = debug_callback,
          .pUserData       = nullptr
     };

     VkDebugUtilsMessengerEXT debug_messenger {};
     if (vkCreateDebugUtilsMessengerEXT(instance, &debug_messenger_info, nullptr, &debug_messenger) != VK_SUCCESS)
          return 6;

     while(!should_quit) {
          MSG message;
          while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
               TranslateMessage(&message);
               DispatchMessageA(&message);
          }
     }

     vkDestroyInstance(instance, nullptr);
     DestroyWindow(hwnd);

     return 0;
}
