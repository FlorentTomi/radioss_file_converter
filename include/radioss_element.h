#ifndef _RADIOSS_ELEMENT_H_
#define _RADIOSS_ELEMENT_H_

#include "io/property_tree.h"

#include <array>
#include <map>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

namespace radioss
{
template<std::size_t Dimension, std::size_t ConnectivitySize, std::size_t TensorSize>
struct Element
{
  using connectivity_t =
      std::conditional_t<ConnectivitySize == 1, std::int32_t,
                         std::array<std::int32_t, ConnectivitySize>>;
  using scalar_functions_t = std::map<std::string, float>;
  using tensor_t = std::array<float, TensorSize>;
  using tensors_t = std::map<std::string, tensor_t>;

  constexpr auto dimension() const -> std::size_t { return Dimension; }
  constexpr auto connectivity_size() const -> std::size_t { return ConnectivitySize; }
  constexpr auto tensor_size() const -> std::size_t { return TensorSize; }

  connectivity_t connectivity = {};
  bool deleted = {};
  scalar_functions_t scalar_functions;
  tensors_t tensors;
  std::optional<float> mass = {};
  std::optional<std::int32_t> internal_number = {};

  auto add_to_ptree(boost::property_tree::ptree& ptree,
                    std::string const& key) const -> void;
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

#include "details/radioss_element.inl.h"

#endif // _RADIOSS_ELEMENT_H_