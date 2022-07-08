#pragma once

#include <volk.h>

#include "core.hpp"
#include "win32.hpp"

class surface {
  public:
     surface(core* engine_core, win32* platform, win32::window* window);
     ~surface();

  private:
     core*          engine_core_;
     win32*         platform_;
     win32::window* platform_window_;
     VkSurfaceKHR   surface_;
};
