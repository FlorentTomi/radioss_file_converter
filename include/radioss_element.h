#ifndef _RADIOSS_ELEMENT_H_
#define _RADIOSS_ELEMENT_H_

#include "io/property_tree.h"

#include <array>
#include <ios>
#include <map>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

namespace radioss
{
template<std::size_t Dimension, std::size_t ConnectivitySize,
         std::size_t TensorSize>
struct Element
{
  using connectivity_t =
      std::conditional_t<ConnectivitySize == 1, std::int32_t,
                         std::array<std::int32_t, ConnectivitySize>>;
  using scalar_functions_t = std::map<std::string, float>;
  using tensor_t = std::array<float, TensorSize>;
  using tensors_t = std::map<std::string, tensor_t>;

  static constexpr auto dimension() -> std::size_t { return Dimension; }
  static constexpr auto connectivity_size() -> std::size_t
  {
    return ConnectivitySize;
  }
  static constexpr auto tensor_size() -> std::size_t { return TensorSize; }

  connectivity_t connectivity = {};
  bool deleted = {};
  scalar_functions_t scalar_functions;
  tensors_t tensors;
  std::optional<float> mass = {};
  std::optional<std::int32_t> internal_number = {};

  auto add_to_ptree(boost::property_tree::ptree& ptree,
                    std::string const& key) const -> void;
  auto add_to_csv(std::ostream& stream) const -> void;
};

using ElementSPH = Element<0, 1, 6>;
using Element2D = Element<2, 4, 3>;
using Element3D = Element<3, 8, 6>;

struct Element1D : Element<1, 2, 9>
{
  std::int32_t skew_number = {};

  auto add_to_ptree(boost::property_tree::ptree& ptree,
                    std::string const& key) const -> void;
};
} // namespace radioss

template<std::size_t Dimension, std::size_t ConnectivitySize,
         std::size_t TensorSize>
auto radioss::Element<Dimension, ConnectivitySize, TensorSize>::add_to_csv(
    std::ostream& stream) const -> void
{
  if constexpr (connectivity_size() == 1)
  {
    stream << connectivity << ",";
  }
  else
  {
    for (auto connectivity_idx : connectivity)
    {
      stream << connectivity_idx << ",";
    }
  }

  stream << std::boolalpha << deleted << ",";

  for (auto const& [_, scalar_function] : scalar_functions)
  {
    stream << scalar_function << ",";
  }

  for (auto const& [_, tensor] : tensors)
  {
    for (auto tensor_val : tensor)
    {
      stream << tensor_val << ",";
    }
  }

  if (mass.has_value())
  {
    stream << mass.value();
  }
  stream << ",";

  if (internal_number.has_value())
  {
    stream << internal_number.value();
  }
}

#include "details/radioss_element.inl.h"

#endif // _RADIOSS_ELEMENT_H_