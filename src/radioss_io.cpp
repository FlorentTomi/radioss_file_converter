#include "radioss_io.h"
#include "io/utils.h"
#include "radioss.h"
#include "radioss_geometry.h"

#include <cstdlib>
#include <exception>
#include <optional>
#include <string>
#include <type_traits>

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <utility>
#include <vector>

static auto suffix_filepath(std::filesystem::path const& filepath,
                            std::string const& suffix) -> std::filesystem::path
{
  std::filesystem::path stem = filepath.stem();
  stem += suffix;

  std::filesystem::path new_filename = stem;
  new_filename += filepath.extension();

  auto new_filepath = filepath;
  new_filepath.remove_filename();
  new_filepath /= new_filename;

  return new_filepath;
}

static auto write_nodes_csv(radioss::Geometry2D const& data,
                            std::filesystem::path const& filepath) -> void
{
  auto const& nodes = data.nodes;
  if (nodes.empty())
  {
    return;
  }

  std::ofstream stream(::suffix_filepath(filepath, "_nodes"));

  stream << "coordinates_x,";
  stream << "coordinates_y,";
  stream << "coordinates_z,";
  stream << "norm_x,";
  stream << "norm_y,";
  stream << "norm_z,";

  for (auto const& [scalar_function_name, _] : nodes.front().scalar_functions)
  {
    stream << scalar_function_name << ",";
  }

  for (auto const& [vector_name, _] : nodes.front().vectors)
  {
    stream << vector_name + "_x,";
    stream << vector_name + "_y,";
    stream << vector_name + "_z,";
  }

  stream << "mass,";
  stream << "internal_number";
  stream << std::endl;

  for (auto const& node : nodes)
  {
    node.add_to_csv(stream);
    stream << std::endl;
  }
}

template<std::size_t Dimension>
static auto get_tensor_suffix(std::size_t idx) -> std::string
{
  if constexpr (Dimension == 2)
  {
    switch (idx)
    {
      case 0:
        return "x";

      case 1:
        return "y";

      case 2:
        return "xy";

      default:
        return std::to_string(idx);
    }
  }
  else if (Dimension == 3)
  {
    switch (idx)
    {
      case 0:
        return "x";

      case 1:
        return "y";

      case 2:
        return "z";

      case 3:
        return "xy";

      case 4:
        return "yz";

      case 5:
        return "zx";

      default:
        return std::to_string(idx);
    }
  }
  else if (Dimension == 1)
  {
    switch (idx)
    {
      case 0:
        return "x";

      case 1:
        return "y";

      case 2:
        return "z";

      case 3:
        return "xy";

      case 4:
        return "yz";

      case 5:
        return "zx";

      default:
        return std::to_string(idx);
    }
  }
  else
  {
    return std::to_string(idx);
  }
}

template<std::size_t Dimension, class Element>
static auto
write_elements_csv(radioss::Geometry<Dimension, Element> const& data,
                   std::filesystem::path const& filepath)
{
  auto const& elements = data.elements;
  if (elements.empty())
  {
    return;
  }

  std::string suffix = "_" + std::to_string(Dimension) + "d";
  std::ofstream stream(::suffix_filepath(filepath, suffix));

  if constexpr (Element::connectivity_size() == 1)
  {
    stream << "node_index,";
  }
  else
  {
    for (std::size_t i = 0; i < Element::connectivity_size(); ++i)
    {
      stream << "node_index_" << i << ",";
    }
  }

  stream << "deleted,";

  for (auto const& [scalar_function_name, _] :
       elements.front().scalar_functions)
  {
    stream << scalar_function_name << ",";
  }

  for (auto const& [tensor_name, _] : elements.front().tensors)
  {
    for (std::size_t i = 0; i < Element::tensor_size(); ++i)
    {
      stream << tensor_name << "_" << ::get_tensor_suffix<Dimension>(i) << ",";
    }
  }

  stream << "mass,";
  stream << "internal_number";
  stream << std::endl;

  for (auto const& element : elements)
  {
    element.add_to_csv(stream);
    stream << std::endl;
  }
}

auto radioss::io::__details::read_data(std::ifstream& stream, std::size_t& data)
    -> void
{
  std::int32_t intData = {};
  radioss::io::__details::read_data(stream, intData);
  data = static_cast<std::size_t>(intData);
}

auto radioss::io::__details::read_data(std::ifstream& stream, std::string& data,
                                       std::size_t size) -> void
{
  data.resize(size);
  stream.read(data.data(), data.size());
  data = radioss::io::utils::fix_name(data);
}

auto radioss::io::__details::read_data(std::ifstream& stream,
                                       radioss::Radioss::flags_t& flags) -> void
{
  std::vector<std::int32_t> intFlags;
  radioss::io::__details::read_data(stream, intFlags, flags.size());

  for (std::size_t i = 0; i < flags.size(); ++i)
  {
    flags.set(i, intFlags.at(i) != 0);
  }
}

auto radioss::io::read(std::filesystem::path const& filepath)
    -> std::optional<radioss::Radioss>
{
  auto data = std::optional<radioss::Radioss>();
  if (!std::filesystem::exists(filepath))
  {
    return data;
  }

  auto stream =
      std::ifstream(filepath, std::ios_base::in | std::ios_base::binary);

  data.emplace();
  radioss::io::__details::read_data(stream, data->format);
  if (!data->valid())
  {
    return data;
  }

  radioss::io::__details::read_data(stream, data->file_time);
  radioss::io::__details::read_data(stream, data->time, 81);
  radioss::io::__details::read_data(stream, data->mod_anim, 81);
  radioss::io::__details::read_data(stream, data->run, 81);
  radioss::io::__details::read_data(stream, data->flags);

  radioss::io::__details::read_data(stream, data->geometry_2d, data->flags);

  if (data->flags.test(2))
  {
    data->geometry_3d.emplace();
    radioss::io::__details::read_data(stream, data->geometry_3d.value(),
                                      data->flags);
  }

  if (data->flags.test(3))
  {
    data->geometry_1d.emplace();
    radioss::io::__details::read_data(stream, data->geometry_1d.value(),
                                      data->flags);
  }

  if (data->flags.test(4))
  {
    std::size_t subset_size = {};
    radioss::io::__details::read_data(stream, subset_size);

    data->subsets.reserve(subset_size);
    for (std::size_t i = 0; i < subset_size; ++i)
    {
      auto& subset = data->subsets.emplace_back();
      radioss::io::__details::read_data(stream, subset.name, 50);
      radioss::io::__details::read_data(stream, subset.parent_number);

      std::size_t subset_son_size = {};
      radioss::io::__details::read_data(stream, subset_son_size);
      radioss::io::__details::read_data(stream, subset.subset_sons,
                                        subset_son_size);

      std::size_t subpart_2d_size = {};
      radioss::io::__details::read_data(stream, subpart_2d_size);
      radioss::io::__details::read_data(stream, subset.subpart_2d,
                                        subpart_2d_size);

      std::size_t subpart_3d_size = {};
      radioss::io::__details::read_data(stream, subpart_3d_size);
      radioss::io::__details::read_data(stream, subset.subpart_3d,
                                        subpart_3d_size);

      std::size_t subpart_1d_size = {};
      radioss::io::__details::read_data(stream, subpart_1d_size);
      radioss::io::__details::read_data(stream, subset.subpart_1d,
                                        subpart_1d_size);
    }

    std::size_t material_size = {};
    radioss::io::__details::read_data(stream, material_size);

    std::size_t property_size = {};
    radioss::io::__details::read_data(stream, property_size);

    std::vector<std::string> material_names;
    radioss::io::__details::read_data(stream, material_names, material_size,
                                      50);

    std::vector<std::int32_t> material_types;
    radioss::io::__details::read_data(stream, material_types, material_size);

    for (std::size_t i = 0; i < material_size; ++i)
    {
      data->materials.emplace(material_names.at(i), material_types.at(i));
    }

    std::vector<std::string> property_names;
    radioss::io::__details::read_data(stream, property_names, property_size,
                                      50);

    std::vector<std::int32_t> property_types;
    radioss::io::__details::read_data(stream, property_types, property_size);

    for (std::size_t i = 0; i < property_size; ++i)
    {
      data->materials.emplace(property_names.at(i), property_types.at(i));
    }
  }

  if (data->flags.test(5))
  {
    auto& time_history = data->time_history.emplace();

    std::size_t node_size = {};
    radioss::io::__details::read_data(stream, node_size);

    std::size_t element_2d_size = {};
    radioss::io::__details::read_data(stream, element_2d_size);

    std::size_t element_3d_size = {};
    radioss::io::__details::read_data(stream, element_3d_size);

    std::size_t element_1d_size = {};
    radioss::io::__details::read_data(stream, element_1d_size);

    std::vector<std::int32_t> node_numbers;
    radioss::io::__details::read_data(stream, node_numbers, node_size);

    std::vector<std::string> node_names;
    radioss::io::__details::read_data(stream, node_names, node_size, 50);

    time_history.nodes.reserve(node_size);
    for (std::size_t i = 0; i < node_size; ++i)
    {
      auto& node = time_history.nodes.emplace_back();
      node.name = node_names.at(i);
      node.internal_number = node_numbers.at(i);
    }

    std::vector<std::int32_t> element_2d_numbers;
    radioss::io::__details::read_data(stream, element_2d_numbers,
                                      element_2d_size);

    std::vector<std::string> element_2d_names;
    radioss::io::__details::read_data(stream, element_2d_names, element_2d_size,
                                      50);

    time_history.elements_2d.reserve(element_2d_size);
    for (std::size_t i = 0; i < element_2d_size; ++i)
    {
      auto& element = time_history.elements_2d.emplace_back();
      element.name = element_2d_names.at(i);
      element.internal_number = element_2d_numbers.at(i);
    }

    std::vector<std::int32_t> element_3d_numbers;
    radioss::io::__details::read_data(stream, element_3d_numbers,
                                      element_3d_size);

    std::vector<std::string> element_3d_names;
    radioss::io::__details::read_data(stream, element_3d_names, element_3d_size,
                                      50);

    time_history.elements_3d.reserve(element_3d_size);
    for (std::size_t i = 0; i < element_3d_size; ++i)
    {
      auto& element = time_history.elements_3d.emplace_back();
      element.name = element_3d_names.at(i);
      element.internal_number = element_3d_numbers.at(i);
    }

    std::vector<std::int32_t> element_1d_numbers;
    radioss::io::__details::read_data(stream, element_1d_numbers,
                                      element_1d_size);

    std::vector<std::string> element_1d_names;
    radioss::io::__details::read_data(stream, element_1d_names, element_1d_size,
                                      50);

    time_history.elements_1d.reserve(element_1d_size);
    for (std::size_t i = 0; i < element_1d_size; ++i)
    {
      auto& element = time_history.elements_1d.emplace_back();
      element.name = element_1d_names.at(i);
      element.internal_number = element_1d_numbers.at(i);
    }
  }

  if (data->flags.test(7))
  {
    // TODO
  }

  return data;
}

auto radioss::io::write_csv(radioss::Radioss const& data,
                            std::filesystem::path const& filepath) -> void
{
  ::write_nodes_csv(data.geometry_2d, filepath);
  ::write_elements_csv(data.geometry_2d, filepath);

  if (data.geometry_3d.has_value())
  {
    ::write_elements_csv(data.geometry_3d.value(), filepath);
  }
  
  if (data.geometry_1d.has_value())
  {
    ::write_elements_csv(data.geometry_1d.value(), filepath);
  }
  
  if (data.geometry_sph.has_value())
  {
    ::write_elements_csv(data.geometry_sph.value(), filepath);
  }
}