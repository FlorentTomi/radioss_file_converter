#ifndef _RADIOSS_H_
#define _RADIOSS_H_

#include "radioss_element.h"
#include "radioss_geometry.h"
#include "radioss_node.h"

#include <boost/property_tree/ptree.hpp>

#include <vtkSmartPointer.h>

#include <bitset>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include <vtkUnstructuredGrid.h>

namespace radioss
{
struct TimeHistory
{
  struct Token
  {
    std::string name;
    std::int32_t internal_number = {};

    auto add_to_ptree(boost::property_tree::ptree& ptree,
                      std::string const& key) const -> void;
  };

  std::vector<Token> nodes;
  std::vector<Token> elements_2d;
  std::vector<Token> elements_3d;
  std::vector<Token> elements_1d;

  auto add_to_ptree(boost::property_tree::ptree& ptree,
                    std::string const& key) const -> void;
};

struct Subset
{
  std::string name;
  std::int32_t parent_number = {};
  std::vector<std::int32_t> subset_sons;
  std::vector<std::int32_t> subpart_2d;
  std::vector<std::int32_t> subpart_3d;
  std::vector<std::int32_t> subpart_1d;

  auto add_to_ptree(boost::property_tree::ptree& ptree,
                    std::string const& key) const -> void;
};

struct Radioss
{
  static constexpr std::int32_t ValidFormat = 0x542c;
  using flags_t = std::bitset<10>;

  std::int32_t format = {};
  float file_time = {};
  std::string time;
  std::string mod_anim;
  std::string run;
  flags_t flags;

  radioss::Geometry2D geometry_2d;
  std::optional<radioss::Geometry3D> geometry_3d;
  std::optional<radioss::Geometry1D> geometry_1d;
  std::optional<radioss::TimeHistory> time_history;
  std::optional<radioss::GeometrySPH> geometry_sph;
  std::vector<radioss::Subset> subsets;
  std::map<std::string, std::int32_t> materials;
  std::map<std::string, std::int32_t> properties;

  inline auto valid() const -> bool { return format == ValidFormat; }
  auto to_tree() const -> boost::property_tree::ptree;
  auto to_unstructured_grid() const -> vtkSmartPointer<vtkUnstructuredGrid>;
};
} // namespace radioss

#endif // _RADIOSS_H_