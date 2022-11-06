#ifndef _RADIOSS_PART_H_
#define _RADIOSS_PART_H_

#include <boost/property_tree/ptree.hpp>

#include <cstdint>
#include <string>

namespace radioss
{
struct Part
{
  std::int32_t definition = {};
  std::string name;
  std::int32_t subset = {};
  std::int32_t material = {};
  std::int32_t property = {};

  auto add_to_ptree(boost::property_tree::ptree& ptree,
                    std::string const& key) const -> void;
};
} // namespace radioss

#endif // _RADIOSS_PART_H_