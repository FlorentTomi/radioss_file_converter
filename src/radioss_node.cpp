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

auto radioss::Node2D::add_to_csv(std::ostream& stream) const -> void
{
  stream << coordinates[0] << ",";
  stream << coordinates[1] << ",";
  stream << coordinates[2] << ",";
  stream << norm[0] << ",";
  stream << norm[1] << ",";
  stream << norm[2] << ",";
  
  for (auto const& [_, scalar_function] : scalar_functions)
  {
    stream << scalar_function << ",";
  }
  
  for (auto const& [_, vector] : vectors)
  {
    stream << vector[0] << ",";
    stream << vector[1] << ",";
    stream << vector[2] << ",";
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