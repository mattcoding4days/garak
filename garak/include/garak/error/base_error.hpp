//
// Created by matt on 8/28/22.
//

#ifndef LIBGARAK_ERROR_BASE_ERROR_HPP
#define LIBGARAK_ERROR_BASE_ERROR_HPP

#include <garak/error/source_location.hpp>

namespace garak::error {
/**
 * @brief Base Error that should be inherited from
 */
class IGarakAbstractError : public std::exception {
 public:
  IGarakAbstractError(std::string_view const &msg,
                      SourceLocation &&slc) noexcept;

  ~IGarakAbstractError() override = default;
  IGarakAbstractError(IGarakAbstractError const &) = default;
  IGarakAbstractError(IGarakAbstractError &&) = default;
  IGarakAbstractError &operator=(IGarakAbstractError const &) = default;
  IGarakAbstractError &operator=(IGarakAbstractError &&) noexcept = default;

  [[nodiscard]] const char *what() const noexcept override;
  [[nodiscard]] virtual std::string_view file() const noexcept;
  [[nodiscard]] virtual std::string_view func() const noexcept;
  [[nodiscard]] virtual u32 line() const noexcept;
  [[nodiscard]] virtual std::string_view formatted_info() const noexcept;

 protected:
  std::string mMsg{};
  std::string mFile{};
  std::string mFunc{};
  u32 mLine{};
  std::string mFormattedInfo{};
};
}  // namespace garak::error

#endif  // LIBGARAK_ERROR_BASE_ERROR_HPP
