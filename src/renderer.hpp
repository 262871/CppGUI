#pragma once

#include "win32.hpp"
#include "surface.hpp"

class renderer {
  public:
     renderer(core* engine_core, win32* platform, win32::window* window);

  private:
     surface surface_;
};
