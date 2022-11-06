#include "radioss_element.h"

auto radioss::Element1D::add_to_ptree(boost::property_tree::ptree& ptree,
                                      std::string const& key) const -> void
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

  child.put("skew_number", skew_number);

  ptree.put_child(key, child);
}