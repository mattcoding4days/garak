//
// Created by matt on 8/28/22.
//

#ifndef LIBGARAK_SOURCE_LOCATION_HPP
#define LIBGARAK_SOURCE_LOCATION_HPP

#include <string>
#include <garak/utils/types.hpp>

namespace garak::error {
/**
 * @brief structure that holds run time location information
 */
struct SourceLocation {
  SourceLocation(std::string_view const &file, u32 line,
                 std::string_view const &func) noexcept;

  const std::string mFile{};
  const u32 mLine{};
  const std::string mFunc{};
};

/**
 * \brief Wrapper macro which will guarantee that the accurate file, function
 *  name, and line number will be reported
 */
#define RUNTIME_INFO                 \
  SourceLocation(__FILE__, __LINE__, \
                 static_cast<const char *>(__PRETTY_FUNCTION__))
}  // namespace garak::error

#endif  // LIBGARAK_SOURCE_LOCATION_HPP
