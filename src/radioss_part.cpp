#include "radioss_part.h"

auto radioss::Part::add_to_ptree(boost::property_tree::ptree& ptree,
                                 std::string const& key) const -> void
{
  boost::property_tree::ptree child;
  child.put("definition", definition);
  child.put("name", name);
  child.put("subset", subset);
  child.put("material", material);
  child.put("property", property);
  ptree.put_child(key, child);
}