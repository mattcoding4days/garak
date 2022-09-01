#ifndef LIBGARAK_UTILS_VERSION_HPP
#define LIBGARAK_UTILS_VERSION_HPP

#include <string>

namespace garak::utils {
constexpr auto VMAJOR = 0;
constexpr auto VMINOR = 1;
constexpr auto VPATCH = 0;

/**
 * @brief return the version in string format
 *
 * @returns std::string
 * */
[[maybe_unused]] std::string get_version();
}  // namespace garak::utils

#endif
