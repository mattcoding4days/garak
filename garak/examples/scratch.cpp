#include <garak/bundled/asio/asio.hpp>
#include <garak/version.hpp>
#include <iostream>

/**
 * @brief A scratch file for interactively testing code
 * */
int main() {
  std::cout << "library version: " << garak::get_version() << '\n';
  return 0;
}
