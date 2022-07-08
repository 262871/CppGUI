#include "renderer.hpp"

renderer::renderer(core* engine_core, win32* platform, win32::window* window)
   : surface_(engine_core, platform, window) {
}
