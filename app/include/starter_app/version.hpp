#ifndef STARTER_APP_VERSION_HPP
#define STARTER_APP_VERSION_HPP

/**
 * @file starter_app/version.hpp
 * @author Matt Williams (matt.k.williams@protonmail.com)
 * @brief Adds version support for project, used by Cmake
 * @date 2021-11-19
 */

#include <string>

#define VMAJOR 0
#define VMINOR 1
#define VPATCH 0

namespace starter::app {
/**
 * @brief return the version in string format
 *
 * @returns std::string
 * */
[[maybe_unused]] std::string get_version();
}  // namespace starter::app

#endif
