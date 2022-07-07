#pragma once

#include <volk.h>

#include <stdexcept>

class Instance {
  public:
     Instance();
     ~Instance();
     VkInstance get();
     
  private:
     VkInstance instance_ {};
};
