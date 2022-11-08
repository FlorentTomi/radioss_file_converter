#ifndef _RADIOSS_READER_H_
#define _RADIOSS_READER_H_

#include "radioss.h"

#include <filesystem>
#include <optional>

namespace radioss::io
{
auto read(std::filesystem::path const& filepath)
    -> std::optional<radioss::Radioss>;
auto write_csv(radioss::Radioss const& data,
               std::filesystem::path const& filepath) -> void;
} // namespace radioss::io

#include "details/radioss_io.inl.h"

#endif // _RADIOSS_READER_H_