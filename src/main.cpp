#include <volk.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define FMT_HEADER_ONLY
#include <fmt/format.h>

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void*) {
     if (severity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
          fmt::print(stderr, "Error: {}\n", callback_data->pMessage);
     else if (severity != VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
          fmt::print(stdout, "Varning: {}\n", callback_data->pMessage);
     else
          fmt::print(stdout, "Info: {}\n", callback_data->pMessage);
     return VK_FALSE;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
     glfwInit();
     glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
     glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
     auto window = glfwCreateWindow(800, 600, "Testing glfw and vulkan", nullptr, nullptr);

     if (volkInitialize() != VK_SUCCESS)
          return 1;

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
     auto debug_ext {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
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
          return 2;
     volkLoadInstance(instance);

     uint32_t device_count {};
     if (vkEnumeratePhysicalDevices(instance, &device_count, nullptr) != VK_SUCCESS)
          return 3;
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
          return 4;

     while (!glfwWindowShouldClose(window)) {
          glfwPollEvents();
     }

     vkDestroyInstance(instance, nullptr);
     glfwDestroyWindow(window);
     glfwTerminate();

     return 0;
}
