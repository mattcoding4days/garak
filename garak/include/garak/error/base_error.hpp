//
// Created by matt on 8/28/22.
//

#ifndef LIBGARAK_ERROR_BASE_ERROR_HPP
#define LIBGARAK_ERROR_BASE_ERROR_HPP

#include <garak/error/source_location.hpp>

namespace garak::error {
/**
 * \brief Base Error that should be inherited from
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
  [[nodiscard]] virtual auto file() const noexcept -> std::string_view;
  [[nodiscard]] virtual auto func() const noexcept -> std::string_view;
  [[nodiscard]] virtual auto line() const noexcept -> std::uint32_t;
  [[nodiscard]] virtual auto formatted_info() const noexcept
      -> std::string_view;

 protected:
  std::string mMsg{};
  std::string mFile{};
  std::string mFunc{};
  std::uint32_t mLine{};
  std::string mFormattedInfo{};
};

/**
 * \brief A basic error class
 */
class GarakError : public IGarakAbstractError {
 public:
  GarakError(std::string_view const &msg, SourceLocation &&slc) noexcept;
};
}  // namespace garak::error

#endif  // LIBGARAK_ERROR_BASE_ERROR_HPP
