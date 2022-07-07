#include "vulkan_manager.hpp"
#include "win32.hpp"
#include "window.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
     struct data {
          bool should_quit { false };
     } user;
     vulkan::get_instance_ref();
     auto  w  = window<win32>(L"Vulkan win32", &user);

     while (!user.should_quit) {
          MSG message;
          while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE)) {
               TranslateMessage(&message);
               DispatchMessageW(&message);
          }
     }

     return 0;
}
