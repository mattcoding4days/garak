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

std::string_view IGarakAbstractError::file() const noexcept { return mFile; }

std::string_view IGarakAbstractError::func() const noexcept { return mFunc; }

u32 IGarakAbstractError::line() const noexcept { return mLine; }

std::string_view IGarakAbstractError::formatted_info() const noexcept {
  return mFormattedInfo;
}
}  // namespace garak::error