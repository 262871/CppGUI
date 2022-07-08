#pragma once

#include <windows.h>

#include <volk.h>

#include <vector>

class core {
  public:
     core(std::vector<const char*> const& extensions, std::vector<const char*> const& layers);
     ~core();

     VkAllocationCallbacks* allocator { nullptr };
     VkInstance             instance;
};
