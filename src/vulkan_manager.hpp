#pragma once

#include <volk.h>

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include <stdexcept>

class vulkan {
  public:
     static vulkan& get_instance_ref() {
          static vulkan instance;
          return instance;
     }

  private:
     vulkan() {
          if (volkInitialize() != VK_SUCCESS)
               throw std::runtime_error("call to volkInitialize failed");

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
          if (vkCreateInstance(&instance_create_info, nullptr, &instance_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateInstance failed");

          volkLoadInstance(instance_);

          uint32_t device_count {};
          if (vkEnumeratePhysicalDevices(instance_, &device_count, nullptr) != VK_SUCCESS)
               throw std::runtime_error("call to vkEnumeratePhysicalDevices failed");
          fmt::print("{} devices present\n", device_count);

#ifndef NDEBUG
          VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info {
               .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
               .pNext           = nullptr,
               .flags           = {},
               .messageSeverity = { VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT },
               .messageType     = { VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT },
               .pfnUserCallback = debug_callback,
               .pUserData       = nullptr
          };

          if (vkCreateDebugUtilsMessengerEXT(instance_, &debug_messenger_info, nullptr, &debug_messenger_) != VK_SUCCESS)
               throw std::runtime_error("call to vkCreateDebugUtilsMessengerEXT failed");
#endif
     }
     ~vulkan() {
          vkDestroyInstance(instance_, nullptr);
     }

     vulkan(const vulkan&)            = delete;
     vulkan(vulkan&&)                 = delete;
     vulkan& operator=(const vulkan&) = delete;
     vulkan& operator=(vulkan&&)      = delete;

     static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT level, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* warn, void*) {
          if (level > VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
               fmt::print(stderr, "Error: {}\n", warn->pMessage);
          else if (level != VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
               fmt::print(stdout, "Varning: {}\n", warn->pMessage);
          else
               fmt::print(stdout, "Info: {}\n", warn->pMessage);
          return VK_FALSE;
     }

     VkInstance instance_ {};
#ifndef NDEBUG
     VkDebugUtilsMessengerEXT debug_messenger_ {};
#endif
};