#include <iostream>
#include <starter_app/version.hpp>

/**
 * @brief A scratch file for interactively testing code
 * */
using namespace starter;

int main() {
  std::cout << "Starter app version: " << app::get_version() << '\n';
  return 0;
}
