#include <iostream>
#include <starter_app/version.hpp>
#include <starter_core/version.hpp>

int main() {
  std::cout << "Starter core version: " << starter::core::get_version() << '\n';
  std::cout << "Starter app version: " << starter::app::get_version() << '\n';

  return 0;
}
