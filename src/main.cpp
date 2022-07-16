#include "GUI.hpp"

#include <stdexcept>

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
     GUI gui;
     try
     {
     gui.run();
          /* code */
     }
     catch(const std::exception& e)
     {
          fmt::print("exception: {}\n", e.what());
     }
     
     return 0;
}
