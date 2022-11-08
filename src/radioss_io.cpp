#include "radioss_io.h"
#include "io/utils.h"
#include "radioss.h"

#include <cstdlib>
#include <exception>
#include <optional>
#include <type_traits>

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <utility>
#include <vector>

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
  auto const& geometry_2d = data.geometry_2d;
  auto const& nodes = geometry_2d.nodes;
  if (nodes.empty())
  {
    return;
  }

  std::ofstream stream(filepath);

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