#include "engine.hpp"

engine::engine(win32* platform, win32::window* window)
   : core_(extensions_, layers_)
   , messenger_(&core_)
   , renderer_(&core_, platform, window)
   , device_(&core_) {
}
engine::~engine() {
}
