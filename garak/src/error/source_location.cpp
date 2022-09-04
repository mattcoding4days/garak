//
// Created by matt on 8/28/22.
//
#include <garak/error/source_location.hpp>

namespace garak::error {
SourceLocation::SourceLocation(std::string_view const &file, u32 line,
                               std::string_view const &func) noexcept
    : mFile(file), mLine(line), mFunc(func) {}
}  // namespace garak::error
