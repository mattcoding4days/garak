#include <asio.hpp>
#include <iostream>

#include <garak/utils/module.hpp>
#include <garak/error/module.hpp>

/**
 * @brief A scratch file for interactively testing code
 * */
 using namespace garak;
int main() {
  std::cout << "library version: " << utils::get_version() << '\n';
  return 0;
}
