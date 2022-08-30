#include <garak/garak.hpp>
#include <iostream>

/**
 * @brief A scratch file for interactively testing code
 * */
using namespace garak;
int main() {
  std::cout << "libgarak version: " << utils::get_version() << '\n';
  return 0;
}
