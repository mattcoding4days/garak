#include <garak/version.hpp>

namespace garak {
std::string get_version() {
  return std::string{std::to_string(VMAJOR)}
      .append(".")
      .append(std::to_string(VMINOR))
      .append(".")
      .append(std::to_string(VPATCH));
}
}  // namespace garak
