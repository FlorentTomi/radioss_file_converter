#ifndef _RADIOSS_NODE_H_
#define _RADIOSS_NODE_H_

#include <boost/property_tree/ptree.hpp>

#include <array>
#include <map>
#include <optional>
#include <string>
#include <vector>


namespace radioss
{
struct Node2D
{
  using coordinates_t = std::array<float, 3>;
  using norm_t = std::array<float, 3>;
  using scalar_functions_t = std::map<std::string, float>;
  using vector_t = std::array<float, 3>;
  using vectors_t = std::map<std::string, vector_t>;

  coordinates_t coordinates;
  norm_t norm;
  scalar_functions_t scalar_functions;
  vectors_t vectors;
  std::optional<float> mass;
  std::optional<std::int32_t> internal_number;

  auto add_to_ptree(boost::property_tree::ptree& ptree,
                    std::string const& key) const -> void;
};
} // namespace radioss

#endif // _RADIOSS_NODE_H_