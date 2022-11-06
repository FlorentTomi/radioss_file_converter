#include "radioss_element.h"

#include <boost/property_tree/ptree.hpp>

#include <cstddef>
#include <string>

template<std::size_t Dimension, std::size_t ConnectivitySize, std::size_t TensorSize>
auto radioss::Element<Dimension, ConnectivitySize, TensorSize>::add_to_ptree(
    boost::property_tree::ptree& ptree, std::string const& key) const -> void
{
  boost::property_tree::ptree child;

  child.put("connectivity", connectivity);
  child.put("deleted", deleted);
  ::io::add_to_ptree(child, "scalar_functions", scalar_functions);
  ::io::add_to_ptree(child, "tensors", tensors);

  if (mass.has_value())
  {
    child.put("mass", mass.value());
  }

  if (internal_number.has_value())
  {
    child.put("internal_number", internal_number.value());
  }

  ptree.put_child(key, child);
}