#ifndef LIBGARAK_UTILS_VERSION_HPP
#define LIBGARAK_UTILS_VERSION_HPP

#include <string>

namespace garak::utils {
#define VMAJOR 0
#define VMINOR 1
#define VPATCH 0

/**
 * @brief return the version in string format
 *
 * @returns std::string
 * */
[[maybe_unused]] std::string get_version();
}  // namespace garak::utils

#endif
