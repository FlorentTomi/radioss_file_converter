#include "io/utils.h"

#include <algorithm>
#include <cstring>
#include <iterator>

auto radioss::io::utils::fix_name(std::string const& name) -> std::string
{
  std::string fixed_name = name;
  fixed_name.resize(std::strlen(fixed_name.c_str())); // remove trailing \u0000
  std::replace(std::begin(fixed_name), std::end(fixed_name), ' ', '_');
  return fixed_name;
}
