#include <volk.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define FMT_HEADER_ONLY
#include <fmt/format.h>

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
     fmt::print("{} extensions supported\nVulkan version: {}", extensions, version);

     VkApplicationInfo application_info {
          .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
          .pNext              = nullptr,
          .pApplicationName   = "GUI",
          .applicationVersion = 1,
          .pEngineName        = "CppGUI",
          .engineVersion      = 1,
          .apiVersion         = version
     };
     VkInstanceCreateInfo instance_create_info {
          .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
          .pNext                   = nullptr,
          .flags                   = {},
          .pApplicationInfo        = &application_info,
          .enabledLayerCount       = 0,
          .ppEnabledLayerNames     = nullptr,
          .enabledExtensionCount   = 0,
          .ppEnabledExtensionNames = nullptr
     };
     VkInstance instance {};
     if (vkCreateInstance(&instance_create_info, nullptr, &instance) != VK_SUCCESS)
          return 2;

     while (!glfwWindowShouldClose(window)) {
          glfwPollEvents();
     }

     vkDestroyInstance(instance, nullptr);
     glfwDestroyWindow(window);
     glfwTerminate();

     return 0;
}
