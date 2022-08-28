//
// Created by matt on 8/28/22.
//
#include <garak/error/base_error.hpp>

namespace garak::error {
IGarakAbstractError::IGarakAbstractError(std::string_view const &msg,
                                         SourceLocation &&slc) noexcept
    : mMsg(msg), mFile(slc.mFile), mFunc(slc.mFunc), mLine(slc.mLine) {
  mFormattedInfo.append("Error: ")
      .append(msg)
      .append("\nFunction: ")
      .append(mFunc)
      .append("\nFile: ")
      .append(mFile)
      .append(":")
      .append(std::to_string(mLine));
}

const char *IGarakAbstractError::what() const noexcept { return mMsg.data(); }

auto IGarakAbstractError::file() const noexcept -> std::string_view {
  return mFile;
}

auto IGarakAbstractError::func() const noexcept -> std::string_view {
  return mFunc;
}

auto IGarakAbstractError::line() const noexcept -> std::uint32_t {
  return mLine;
}

auto IGarakAbstractError::formatted_info() const noexcept -> std::string_view {
  return mFormattedInfo;
}

GarakError::GarakError(std::string_view const &msg,
                       SourceLocation &&slc) noexcept
    : IGarakAbstractError(msg, std::move(slc)) {}
}  // namespace garak::error