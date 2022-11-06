#ifndef _RADIOSS_GEOMETRY_H_
#define _RADIOSS_GEOMETRY_H_

#include "radioss_element.h"
#include "radioss_node.h"
#include "radioss_part.h"

#include <boost/property_tree/ptree.hpp>

#include <vtkUnstructuredGrid.h>

#include <array>
#include <bitset>
#include <fstream>
#include <vector>

namespace radioss
{
template<std::size_t Dimension, class Element>
struct Geometry
{
  using element_t = Element;
  
  constexpr auto dimension() const -> std::size_t { return Dimension; }

  std::vector<element_t> elements;
  std::vector<radioss::Part> parts;

  auto add_to_ptree(boost::property_tree::ptree& ptree,
                    std::string const& key) const -> void;

  auto add_to_unstructured_grid(vtkUnstructuredGrid& unstructured_grid) const
      -> void;
};

using GeometrySPH = radioss::Geometry<0, radioss::ElementSPH>;
using Geometry1D = radioss::Geometry<1, radioss::Element1D>;

struct Geometry2D : Geometry<2, radioss::Element2D>
{
  using skew_t = std::array<float, 6>;

  std::vector<skew_t> skews;
  std::vector<radioss::Node2D> nodes;

  auto add_to_ptree(boost::property_tree::ptree& ptree,
                    std::string const& key) const -> void;

  auto add_to_unstructured_grid(vtkUnstructuredGrid& unstructured_grid) const
      -> void;
};

using Geometry3D = radioss::Geometry<3, radioss::Element3D>;
} // namespace radioss

#include "details/radioss_geometry.inl.h"

#endif // _RADIOSS_GEOMETRY_H_