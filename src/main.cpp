#include "DebugMessenger.hpp"
#include "Instance.hpp"
#include "win32.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
     auto instance  = Instance();
     auto messenger = DebugMessenger(instance.get());

     auto user     = event_dispatcher();
     auto platform = win32();
     auto window   = platform.make_window(L"Vulkan win32", &user);

     while (!user.should_quit) {
          platform.process_messages();
     }

     return 0;
}
