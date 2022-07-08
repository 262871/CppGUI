#pragma once

#include "core.hpp"

class device {
  public:
     device(core* engine_core);
     ~device();

  private:
     core*            engine_core_;
     VkPhysicalDevice physical_device_;
     VkDevice         device_;
};
