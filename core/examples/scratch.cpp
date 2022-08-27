#include <iostream>
#include <starter_core/version.hpp>

/**
 * @brief A scratch file for interactively testing code
 * */
using namespace starter;

int main() {
  std::cout << "Starter core library version: " << core::get_version() << '\n';
  return 0;
}
