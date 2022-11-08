#include "radioss_geometry.h"
#include "radioss_element.h"
#include "radioss_io.h"
#include "radioss_node.h"

#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkPoints.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <utility>

auto radioss::io::__details::read_data(std::ifstream& stream,
                                       radioss::GeometrySPH& geometry,
                                       std::bitset<10> const& flags) -> void
{
  std::size_t element_size = {};
  radioss::io::__details::read_data(stream, element_size);

  std::size_t part_size = {};
  radioss::io::__details::read_data(stream, part_size);

  std::size_t element_scalar_func_size = {};
  radioss::io::__details::read_data(stream, element_scalar_func_size);

  std::size_t tensor_size = {};
  radioss::io::__details::read_data(stream, tensor_size);

  geometry.elements.resize(element_size);
  geometry.parts.resize(part_size);

  {
    std::vector<radioss::ElementSPH::connectivity_t> element_connectivities;
    radioss::io::__details::read_data(stream, element_connectivities,
                                      element_size);

    std::vector<std::uint8_t> element_deleted;
    radioss::io::__details::read_data(stream, element_deleted, element_size);

    for (std::size_t i = 0; i < element_size; ++i)
    {
      auto& element = geometry.elements.at(i);
      element.connectivity = element_connectivities.at(i);
      element.deleted = element_deleted.at(i) > 0;
    }
  }

  if (part_size > 0)
  {
    std::vector<std::int32_t> part_definitions;
    radioss::io::__details::read_data(stream, part_definitions, part_size);

    std::vector<std::string> part_names;
    radioss::io::__details::read_data(stream, part_names, part_size, 50);

    for (std::size_t i = 0; i < part_size; ++i)
    {
      auto& part = geometry.parts.at(i);
      part.definition = part_definitions.at(i);
      part.name = part_names.at(i);
    }
  }

  if (element_scalar_func_size > 0)
  {
    std::vector<std::vector<std::string>> element_scalar_function_names;
    radioss::io::__details::read_data(stream, element_scalar_function_names,
                                      element_size, element_scalar_func_size,
                                      81);

    std::vector<std::vector<float>> element_scalar_functions;
    radioss::io::__details::read_data(stream, element_scalar_functions,
                                      element_size, element_scalar_func_size);

    for (std::size_t i = 0; i < element_size; ++i)
    {
      auto& element = geometry.elements.at(i);
      auto const& element_scalar_func_names =
          element_scalar_function_names.at(i);
      auto const& element_scalar_funcs = element_scalar_functions.at(i);
      for (std::size_t j = 0; j < element_scalar_func_size; ++j)
      {
        element.scalar_functions.emplace(element_scalar_func_names.at(j),
                                         element_scalar_funcs.at(j));
      }
    }
  }

  if (tensor_size > 0)
  {
    std::vector<std::string> tensor_names;
    radioss::io::__details::read_data(stream, tensor_names, tensor_size, 81);

    for (std::size_t i = 0; i < element_size; ++i)
    {
      auto& element = geometry.elements.at(i);
      for (auto const& tensor_name : tensor_names)
      {
        radioss::ElementSPH::tensor_t tensor_arr;
        radioss::io::__details::read_data(stream, tensor_arr);
        element.tensors.emplace(tensor_name, tensor_arr);
      }
    }
  }

  if (flags.test(0))
  {
    std::vector<float> element_masses;
    radioss::io::__details::read_data(stream, element_masses, element_size);
    for (std::size_t i = 0; i < element_size; ++i)
    {
      geometry.elements.at(i).mass.emplace(element_masses.at(i));
    }
  }

  if (flags.test(1))
  {
    std::vector<std::int32_t> element_number;
    radioss::io::__details::read_data(stream, element_number, element_size);
    for (std::size_t i = 0; i < element_size; ++i)
    {
      geometry.elements.at(i).internal_number.emplace(element_number.at(i));
    }
  }

  if (flags.test(4))
  {
    geometry.parts.resize(part_size);
    for (auto& part : geometry.parts)
    {
      radioss::io::__details::read_data(stream, part.subset);
      radioss::io::__details::read_data(stream, part.material);
      radioss::io::__details::read_data(stream, part.property);
    }
  }
}

auto radioss::io::__details::read_data(std::ifstream& stream,
                                       radioss::Geometry1D& geometry,
                                       std::bitset<10> const& flags) -> void
{
  std::size_t element_size = {};
  radioss::io::__details::read_data(stream, element_size);

  std::size_t part_size = {};
  radioss::io::__details::read_data(stream, part_size);

  std::size_t element_scalar_func_size = {};
  radioss::io::__details::read_data(stream, element_scalar_func_size);

  std::size_t tensor_size = {};
  radioss::io::__details::read_data(stream, tensor_size);

  std::size_t skew_size = {};
  radioss::io::__details::read_data(stream, skew_size);

  geometry.elements.resize(element_size);
  geometry.parts.resize(part_size);

  {
    std::vector<radioss::Element1D::connectivity_t> element_connectivities;
    radioss::io::__details::read_data(stream, element_connectivities,
                                      element_size);

    std::vector<std::uint8_t> element_deleted;
    radioss::io::__details::read_data(stream, element_deleted, element_size);

    for (std::size_t i = 0; i < element_size; ++i)
    {
      auto& element = geometry.elements.at(i);
      element.connectivity = element_connectivities.at(i);
      element.deleted = element_deleted.at(i) > 0;
    }
  }

  if (part_size > 0)
  {
    std::vector<std::int32_t> part_definitions;
    radioss::io::__details::read_data(stream, part_definitions, part_size);

    std::vector<std::string> part_names;
    radioss::io::__details::read_data(stream, part_names, part_size, 50);

    for (std::size_t i = 0; i < part_size; ++i)
    {
      auto& part = geometry.parts.at(i);
      part.definition = part_definitions.at(i);
      part.name = part_names.at(i);
    }
  }

  if (element_scalar_func_size > 0)
  {
    std::vector<std::vector<std::string>> element_scalar_function_names;
    radioss::io::__details::read_data(stream, element_scalar_function_names,
                                      element_size, element_scalar_func_size,
                                      81);

    std::vector<std::vector<float>> element_scalar_functions;
    radioss::io::__details::read_data(stream, element_scalar_functions,
                                      element_size, element_scalar_func_size);

    for (std::size_t i = 0; i < element_size; ++i)
    {
      auto& element = geometry.elements.at(i);
      auto const& element_scalar_func_names =
          element_scalar_function_names.at(i);
      auto const& element_scalar_funcs = element_scalar_functions.at(i);
      for (std::size_t j = 0; j < element_scalar_func_size; ++j)
      {
        element.scalar_functions.emplace(element_scalar_func_names.at(j),
                                         element_scalar_funcs.at(j));
      }
    }
  }

  if (tensor_size > 0)
  {
    std::vector<std::string> tensor_names;
    radioss::io::__details::read_data(stream, tensor_names, tensor_size, 81);

    for (std::size_t i = 0; i < element_size; ++i)
    {
      auto& element = geometry.elements.at(i);
      for (auto const& tensor_name : tensor_names)
      {
        radioss::Element1D::tensor_t tensor_arr;
        radioss::io::__details::read_data(stream, tensor_arr);
        element.tensors.emplace(tensor_name, tensor_arr);
      }
    }
  }

  if (skew_size > 0)
  {
    std::vector<std::int32_t> skews;
    radioss::io::__details::read_data(stream, skews, element_size);
    for (std::size_t i = 0; i < element_size; ++i)
    {
      auto& element = geometry.elements.at(i);
      element.skew_number = skews.at(i);
    }
  }

  if (flags.test(0))
  {
    std::vector<float> element_masses;
    radioss::io::__details::read_data(stream, element_masses, element_size);
    for (std::size_t i = 0; i < element_size; ++i)
    {
      geometry.elements.at(i).mass.emplace(element_masses.at(i));
    }
  }

  if (flags.test(1))
  {
    std::vector<std::int32_t> element_number;
    radioss::io::__details::read_data(stream, element_number, element_size);
    for (std::size_t i = 0; i < element_size; ++i)
    {
      geometry.elements.at(i).internal_number.emplace(element_number.at(i));
    }
  }

  if (flags.test(4))
  {
    geometry.parts.resize(part_size);
    for (auto& part : geometry.parts)
    {
      radioss::io::__details::read_data(stream, part.subset);
      radioss::io::__details::read_data(stream, part.material);
      radioss::io::__details::read_data(stream, part.property);
    }
  }
}

auto radioss::io::__details::read_data(std::ifstream& stream,
                                       radioss::Geometry2D& geometry,
                                       std::bitset<10> const& flags) -> void
{
  std::size_t node_size = {};
  radioss::io::__details::read_data(stream, node_size);

  std::size_t element_size = {};
  radioss::io::__details::read_data(stream, element_size);

  std::size_t part_size = {};
  radioss::io::__details::read_data(stream, part_size);

  std::size_t node_scalar_func_size = {};
  radioss::io::__details::read_data(stream, node_scalar_func_size);

  std::size_t element_scalar_func_size = {};
  radioss::io::__details::read_data(stream, element_scalar_func_size);

  std::size_t vector_size = {};
  radioss::io::__details::read_data(stream, vector_size);

  std::size_t tensor_size = {};
  radioss::io::__details::read_data(stream, tensor_size);

  std::size_t skew_size = {};
  radioss::io::__details::read_data(stream, skew_size);

  {
    using short_skew_t =
        std::array<uint16_t, std::tuple_size_v<radioss::Geometry2D::skew_t>>;

    std::vector<short_skew_t> short_skews;
    radioss::io::__details::read_data(stream, short_skews, skew_size);

    geometry.skews.reserve(skew_size);
    for (auto short_skew : short_skews)
    {
      auto& skew = geometry.skews.emplace_back();
      for (std::size_t i = 0; i < skew.size(); ++i)
      {
        static constexpr float ConversionRatio = 1.0f / 3000.0f;
        skew[i] = static_cast<float>(short_skew[i]) * ConversionRatio;
      }
    }
  }

  geometry.nodes.resize(node_size);
  geometry.elements.resize(element_size);
  geometry.parts.resize(part_size);

  {
    std::vector<radioss::Node2D::coordinates_t> node_coordinates;
    radioss::io::__details::read_data(stream, node_coordinates, node_size);
    for (std::size_t i = 0; i < node_size; ++i)
    {
      auto& node = geometry.nodes.at(i);
      node.coordinates = node_coordinates.at(i);
    }
  }

  if (element_size > 0)
  {
    std::vector<radioss::Element2D::connectivity_t> element_connectivities;
    radioss::io::__details::read_data(stream, element_connectivities,
                                      element_size);

    std::vector<char> element_deleted;
    radioss::io::__details::read_data(stream, element_deleted, element_size);

    for (std::size_t i = 0; i < element_size; ++i)
    {
      auto& element = geometry.elements.at(i);
      element.connectivity = element_connectivities.at(i);
      element.deleted = (element_deleted.at(i) > 0);
    }
  }

  if (part_size > 0)
  {
    std::vector<std::int32_t> part_definitions;
    radioss::io::__details::read_data(stream, part_definitions, part_size);

    std::vector<std::string> part_names;
    radioss::io::__details::read_data(stream, part_names, part_size, 50);

    for (std::size_t i = 0; i < part_size; ++i)
    {
      auto& part = geometry.parts.at(i);
      part.definition = part_definitions.at(i);
      part.name = part_names.at(i);
    }
  }

  {
    using short_norm_t =
        std::array<uint16_t, std::tuple_size_v<radioss::Node2D::norm_t>>;

    std::vector<short_norm_t> short_norms;
    radioss::io::__details::read_data(stream, short_norms, node_size);

    for (std::size_t i = 0; i < node_size; ++i)
    {
      auto& node = geometry.nodes.at(i);
      auto const& short_norm = short_norms.at(i);
      for (std::size_t j = 0; j < node.norm.size(); ++j)
      {
        static constexpr float ConversionRatio = 1.0f / 3000.0f;
        node.norm[j] = static_cast<float>(short_norm[j]) * ConversionRatio;
      }
    }
  }

  std::size_t total_scalar_func_size =
      node_scalar_func_size + element_scalar_func_size;
  if (total_scalar_func_size > 0)
  {
    std::vector<std::vector<std::string>> node_scalar_function_names;
    radioss::io::__details::read_data(stream, node_scalar_function_names,
                                      node_size, node_scalar_func_size, 81);

    std::vector<std::vector<std::string>> element_scalar_function_names;
    radioss::io::__details::read_data(stream, element_scalar_function_names,
                                      element_size, element_scalar_func_size,
                                      81);

    std::vector<std::vector<float>> node_scalar_functions;
    radioss::io::__details::read_data(stream, node_scalar_functions, node_size,
                                      node_scalar_func_size);

    std::vector<std::vector<float>> element_scalar_functions;
    radioss::io::__details::read_data(stream, element_scalar_functions,
                                      element_size, element_scalar_func_size);

    for (std::size_t i = 0; i < node_size; ++i)
    {
      auto& node = geometry.nodes.at(i);
      auto const& node_scalar_func_names = node_scalar_function_names.at(i);
      auto const& node_scalar_funcs = node_scalar_functions.at(i);
      for (std::size_t j = 0; j < node_scalar_func_size; ++j)
      {
        node.scalar_functions.emplace(node_scalar_func_names.at(j),
                                      node_scalar_funcs.at(j));
      }
    }

    for (std::size_t i = 0; i < element_size; ++i)
    {
      auto& element = geometry.elements.at(i);
      auto const& element_scalar_func_names =
          element_scalar_function_names.at(i);
      auto const& element_scalar_funcs = element_scalar_functions.at(i);
      for (std::size_t j = 0; j < element_scalar_func_size; ++j)
      {
        element.scalar_functions.emplace(element_scalar_func_names.at(j),
                                         element_scalar_funcs.at(j));
      }
    }
  }

  if (vector_size > 0)
  {
    std::vector<std::string> vector_names;
    radioss::io::__details::read_data(stream, vector_names, vector_size, 81);

    for (std::size_t i = 0; i < node_size; ++i)
    {
      auto& node = geometry.nodes.at(i);
      for (auto const& vector_name : vector_names)
      {
        radioss::Node2D::vector_t vector_arr;
        radioss::io::__details::read_data(stream, vector_arr);
        node.vectors.emplace(vector_name, vector_arr);
      }
    }
  }

  if (tensor_size > 0)
  {
    std::vector<std::string> tensor_names;
    radioss::io::__details::read_data(stream, tensor_names, tensor_size, 81);

    for (std::size_t i = 0; i < element_size; ++i)
    {
      auto& element = geometry.elements.at(i);
      for (auto const& tensor_name : tensor_names)
      {
        radioss::Element2D::tensor_t tensor_arr;
        radioss::io::__details::read_data(stream, tensor_arr);
        element.tensors.emplace(tensor_name, tensor_arr);
      }
    }
  }

  if (flags.test(0))
  {
    std::vector<float> element_masses;
    radioss::io::__details::read_data(stream, element_masses, element_size);
    for (std::size_t i = 0; i < element_size; ++i)
    {
      geometry.elements.at(i).mass.emplace(element_masses.at(i));
    }

    std::vector<float> node_masses;
    radioss::io::__details::read_data(stream, node_masses, node_size);
    for (std::size_t i = 0; i < node_size; ++i)
    {
      geometry.nodes.at(i).mass.emplace(node_masses.at(i));
    }
  }

  if (flags.test(1))
  {
    std::vector<std::int32_t> node_number;
    radioss::io::__details::read_data(stream, node_number, node_size);
    for (std::size_t i = 0; i < node_size; ++i)
    {
      geometry.nodes.at(i).internal_number.emplace(node_number.at(i));
    }

    std::vector<std::int32_t> element_number;
    radioss::io::__details::read_data(stream, element_number, element_size);
    for (std::size_t i = 0; i < element_size; ++i)
    {
      geometry.elements.at(i).internal_number.emplace(element_number.at(i));
    }
  }

  if (flags.test(4))
  {
    geometry.parts.resize(part_size);
    for (auto& part : geometry.parts)
    {
      radioss::io::__details::read_data(stream, part.subset);
      radioss::io::__details::read_data(stream, part.material);
      radioss::io::__details::read_data(stream, part.property);
    }
  }
}

auto radioss::io::__details::read_data(std::ifstream& stream,
                                       radioss::Geometry3D& geometry,
                                       std::bitset<10> const& flags) -> void
{
  std::size_t element_size = {};
  radioss::io::__details::read_data(stream, element_size);

  std::size_t part_size = {};
  radioss::io::__details::read_data(stream, part_size);

  std::size_t element_scalar_func_size = {};
  radioss::io::__details::read_data(stream, element_scalar_func_size);

  std::size_t tensor_size = {};
  radioss::io::__details::read_data(stream, tensor_size);

  geometry.elements.resize(element_size);
  geometry.parts.resize(part_size);

  {
    std::vector<radioss::Element3D::connectivity_t> element_connectivities;
    radioss::io::__details::read_data(stream, element_connectivities,
                                      element_size);

    std::vector<std::uint8_t> element_deleted;
    radioss::io::__details::read_data(stream, element_deleted, element_size);

    for (std::size_t i = 0; i < element_size; ++i)
    {
      auto& element = geometry.elements.at(i);
      element.connectivity = element_connectivities.at(i);
      element.deleted = element_deleted.at(i) > 0;
    }
  }

  if (part_size > 0)
  {
    std::vector<std::int32_t> part_definitions;
    radioss::io::__details::read_data(stream, part_definitions, part_size);

    std::vector<std::string> part_names;
    radioss::io::__details::read_data(stream, part_names, part_size, 50);

    for (std::size_t i = 0; i < part_size; ++i)
    {
      auto& part = geometry.parts.at(i);
      part.definition = part_definitions.at(i);
      part.name = part_names.at(i);
    }
  }

  if (element_scalar_func_size > 0)
  {
    std::vector<std::vector<std::string>> element_scalar_function_names;
    radioss::io::__details::read_data(stream, element_scalar_function_names,
                                      element_size, element_scalar_func_size,
                                      81);

    std::vector<std::vector<float>> element_scalar_functions;
    radioss::io::__details::read_data(stream, element_scalar_functions,
                                      element_size, element_scalar_func_size);

    for (std::size_t i = 0; i < element_size; ++i)
    {
      auto& element = geometry.elements.at(i);
      auto const& element_scalar_func_names =
          element_scalar_function_names.at(i);
      auto const& element_scalar_funcs = element_scalar_functions.at(i);
      for (std::size_t j = 0; j < element_scalar_func_size; ++j)
      {
        element.scalar_functions.emplace(element_scalar_func_names.at(j),
                                         element_scalar_funcs.at(j));
      }
    }
  }

  if (tensor_size > 0)
  {
    std::vector<std::string> tensor_names;
    radioss::io::__details::read_data(stream, tensor_names, tensor_size, 81);

    for (std::size_t i = 0; i < element_size; ++i)
    {
      auto& element = geometry.elements.at(i);
      for (auto const& tensor_name : tensor_names)
      {
        radioss::Element3D::tensor_t tensor_arr;
        radioss::io::__details::read_data(stream, tensor_arr);
        element.tensors.emplace(tensor_name, tensor_arr);
      }
    }
  }

  if (flags.test(0))
  {
    std::vector<float> element_masses;
    radioss::io::__details::read_data(stream, element_masses, element_size);
    for (std::size_t i = 0; i < element_size; ++i)
    {
      geometry.elements.at(i).mass.emplace(element_masses.at(i));
    }
  }

  if (flags.test(1))
  {
    std::vector<std::int32_t> element_number;
    radioss::io::__details::read_data(stream, element_number, element_size);
    for (std::size_t i = 0; i < element_size; ++i)
    {
      geometry.elements.at(i).internal_number.emplace(element_number.at(i));
    }
  }

  if (flags.test(4))
  {
    geometry.parts.resize(part_size);
    for (auto& part : geometry.parts)
    {
      radioss::io::__details::read_data(stream, part.subset);
      radioss::io::__details::read_data(stream, part.material);
      radioss::io::__details::read_data(stream, part.property);
    }
  }
}

auto radioss::Geometry2D::add_to_ptree(boost::property_tree::ptree& ptree,
                                       std::string const& key) const -> void
{
  boost::property_tree::ptree child;

  boost::property_tree::ptree element_ptree;
  for (auto const& element : elements)
  {
    boost::property_tree::ptree element_child;
    element.add_to_ptree(element_child, "element");
    element_ptree.push_back(std::make_pair("", element_child));
  }
  child.put_child("elements", element_ptree);

  boost::property_tree::ptree part_ptree;
  for (auto const& part : parts)
  {
    boost::property_tree::ptree part_child;
    part.add_to_ptree(part_child, "part");
    part_ptree.push_back(std::make_pair("", part_child));
  }
  child.put_child("parts", part_ptree);

  boost::property_tree::ptree skew_ptree;
  for (auto const& skew : skews)
  {
    // TODO
    // skew_ptree.put("", skew);
  }
  child.put_child("skews", skew_ptree);

  boost::property_tree::ptree node_ptree;
  for (auto const& node : nodes)
  {
    boost::property_tree::ptree node_child;
    node.add_to_ptree(node_child, "node");
    node_ptree.push_back(std::make_pair("", node_child));
  }
  child.put_child("nodes", node_ptree);

  ptree.put_child(key, child);
}

auto radioss::Geometry2D::add_to_unstructured_grid(
    vtkUnstructuredGrid& unstructured_grid) const -> void
{
  auto points = vtkNew<vtkPoints>();
  auto point_data = unstructured_grid.GetPointData();

  auto node_numbers = vtkNew<vtkIntArray>();
  node_numbers->SetName("NODE_ID");
  node_numbers->SetNumberOfComponents(1);

  for (auto const& node : nodes)
  {
    points->InsertNextPoint(node.coordinates.at(0), node.coordinates.at(1),
                            node.coordinates.at(2));

    node_numbers->InsertNextValue(node.internal_number.value_or(-1));

    for (auto const& [scalar_function_name, scalar_function] : node.scalar_functions)
    {
      auto fixed_scalar_function_name = radioss::io::utils::fix_name(scalar_function_name);
      auto* scalar_function_data = point_data->GetArray(fixed_scalar_function_name.c_str());
      if (scalar_function_data == nullptr)
      {
        auto new_scalar_function_data = vtkNew<vtkFloatArray>();
        new_scalar_function_data->SetName(fixed_scalar_function_name.c_str());
        new_scalar_function_data->SetNumberOfComponents(1);

        point_data->AddArray(new_scalar_function_data);
        scalar_function_data = new_scalar_function_data;
      }

      static_cast<vtkFloatArray*>(scalar_function_data)->InsertNextValue(scalar_function);
    }
    
    for (auto const& [vector_name, vector] : node.vectors)
    {
      auto fixed_vector_name = radioss::io::utils::fix_name(vector_name);
      auto* vector_data = point_data->GetArray(fixed_vector_name.c_str());
      if (vector_data == nullptr)
      {
        auto new_vector_data = vtkNew<vtkFloatArray>();
        new_vector_data->SetName(fixed_vector_name.c_str());
        new_vector_data->SetNumberOfComponents(3);

        point_data->AddArray(new_vector_data);
        vector_data = new_vector_data;
      }

      static_cast<vtkFloatArray*>(vector_data)->InsertNextTuple3(vector.at(0), vector.at(1), vector.at(2));
    }
  }

  unstructured_grid.SetPoints(points);
  point_data->AddArray(node_numbers);

  Geometry<2, element_t>::add_to_unstructured_grid(unstructured_grid);
}
