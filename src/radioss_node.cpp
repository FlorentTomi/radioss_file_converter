#include "radioss_node.h"
#include "io/property_tree.h"

auto radioss::Node2D::add_to_ptree(boost::property_tree::ptree& ptree,
                                   std::string const& key) const -> void
{
  boost::property_tree::ptree child;

  ptree.put("coordinates", coordinates);
  ptree.put("norm", norm);
  ::io::add_to_ptree(ptree, "scalar_functions", scalar_functions);
  ::io::add_to_ptree(ptree, "vectors", vectors);

  if (mass.has_value())
  {
    ptree.put("mass", mass.value());
  }

  if (internal_number.has_value())
  {
    ptree.put("internal_number", internal_number.value());
  }

  ptree.put_child(key, child);
}