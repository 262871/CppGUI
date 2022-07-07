#pragma once

#include <volk.h>

#include <stdexcept>

class instance {
  public:
     instance();
     ~instance();
     VkInstance get();
     
  private:
     VkInstance instance_ {};
};
