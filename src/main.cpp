
#include "radioss.h"
#include "radioss_io.h"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <winsock2.h>

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <vtkUnstructuredGridWriter.h>

#include <array>
#include <exception>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <istream>
#include <regex>
#include <sstream>
#include <stdint.h>
#include <string>
#include <string_view>

static auto validate_file(std::filesystem::path const& filepath) -> bool
{
  static constexpr std::string_view RadiossAnimRegex = R"(^.*A\d{3}$)";

  if (!std::filesystem::exists(filepath))
  {
    return false;
  }

  if (filepath.has_extension())
  {
    return false;
  }

  std::regex const re{RadiossAnimRegex.data()};
  return std::regex_match(filepath.stem().string(), re);
}

static auto save_as_json(radioss::Radioss const& data,
                         std::filesystem::path const& filepath) -> void
{
  std::ofstream json_stream(filepath);
  boost::property_tree::json_parser::write_json(json_stream, data.to_tree());
}

static auto save_as_vtk(radioss::Radioss const& data,
                        std::filesystem::path const& filepath) -> void
{
  auto unstructured_grid = data.to_unstructured_grid();

  auto writer = vtkNew<vtkUnstructuredGridWriter>();
  writer->SetInputData(unstructured_grid);
  writer->SetFileName(filepath.string().c_str());
  writer->Update();
}

static auto save_as_csv(radioss::Radioss const& data,
                        std::filesystem::path const& filepath) -> void
{
  radioss::io::write_csv(data, filepath);
}

using save_function_type =
    std::function<void(radioss::Radioss const&, std::filesystem::path const&)>;

static auto get_save_function(std::filesystem::path const& filepath)
    -> save_function_type
{
  if (filepath.extension() == ".json")
  {
    return ::save_as_json;
  }
  else if (filepath.extension() == ".vtk")
  {
    return ::save_as_vtk;
  }
  else if (filepath.extension() == ".csv")
  {
    return ::save_as_csv;
  }

  return nullptr;
}

struct ProgramData
{
  std::filesystem::path filepath;
  std::vector<std::filesystem::path> output_filepath;
};

auto main(int argc, char** argv) -> int
{
  namespace boost_po = boost::program_options;

  auto opt_description = boost_po::options_description{"allowed options"};

  // clang-format off
  opt_description.add_options()
    ("help,h", "produce help message")
    ("input,i", boost_po::value<std::filesystem::path>(), "input file (RunnameAXXX)")
    ("input-directory,I", boost_po::value<std::filesystem::path>(), "directory containing the input files")
    ("output,o", boost_po::value<std::filesystem::path>(), "output directory, input directory if not present")
    ("output-type,t", boost_po::value<std::vector<std::string>>()->required(), "output file type(s) (json, vtk, csv), output files will be output in the same directory");
  // clang-format on

  boost_po::variables_map var_map;
  try
  {
    boost_po::store(boost_po::parse_command_line(argc, argv, opt_description),
                    var_map);
    boost_po::notify(var_map);
  }
  catch (std::exception const&)
  {
    std::cout << "invalid syntax" << std::endl;
    std::cout << opt_description << std::endl;
    return -1;
  }

  if (var_map.count("help") > 0)
  {
    std::cout << opt_description << std::endl;
    return 1;
  }

  auto input_filepaths = std::vector<std::filesystem::path>();

  if (var_map.count("input") > 0)
  {
    auto input_filepath = var_map.at("input").as<std::filesystem::path>();
    if (!::validate_file(input_filepath))
    {
      std::cout << "invalid file: " << input_filepath << std::endl;
    }
    else
    {
      input_filepaths.emplace_back(input_filepath);
    }
  }

  if (var_map.count("input-directory") > 0)
  {
    auto input_directory =
        var_map.at("input-directory").as<std::filesystem::path>();
    if (!std::filesystem::is_directory(input_directory))
    {
      std::cout << input_directory << " is not a valid directory" << std::endl;
    }
    else
    {
      for (auto filepath : std::filesystem::directory_iterator(input_directory))
      {
        if (::validate_file(filepath))
        {
          input_filepaths.emplace_back(filepath);
        }
      }
    }
  }

  if (input_filepaths.empty())
  {
    std::cout << "no valid input file" << std::endl;
    return -1;
  }

  std::filesystem::path output_directory;
  if (var_map.count("output") > 0)
  {
    output_directory = var_map.at("output").as<std::filesystem::path>();
  }

  std::vector<ProgramData> program_data;
  if (var_map.count("output-type") > 0)
  {
    auto output_types =
        var_map.at("output-type").as<std::vector<std::string>>();
    std::transform(std::begin(output_types), std::end(output_types),
                   std::begin(output_types),
                   [](std::string const& output_type)
                   {
                     std::string output_type_lower = output_type;
                     std::transform(std::begin(output_type_lower),
                                    std::end(output_type_lower),
                                    std::begin(output_type_lower),
                                    [](char c) -> char
                                    { return std::tolower(c); });
                     return output_type_lower;
                   });

    if (!std::all_of(std::begin(output_types), std::end(output_types),
                     [](std::string const& output_type) -> bool
                     {
                       return (output_type == "json") ||
                              (output_type == "vtk") || (output_type == "csv");
                     }))
    {
      std::cout << "invalid output type" << std::endl;
      return -1;
    }

    for (auto const& input_filepath : input_filepaths)
    {
      auto output_filepath = output_directory.empty()
                                 ? input_filepath
                                 : output_directory / input_filepath.filename();
      std::vector<std::filesystem::path> output_filepaths;
      for (auto const& output_type : output_types)
      {
        output_filepath.replace_extension("." + output_type);
        output_filepaths.emplace_back(output_filepath);
      }

      program_data.emplace_back(ProgramData{input_filepath, output_filepaths});
    }
  }

  for (auto const& [input_filepath, output_filepaths] : program_data)
  {
    std::cout << "\t- converting file: " << input_filepath << std::endl;
    auto data = radioss::io::read(input_filepath);
    if (!data.has_value())
    {
      std::cout << "invalid Radioss file: " << input_filepath << std::endl;
      continue;
    }

    for (auto const& output_filepath : output_filepaths)
    {
      auto save_function = ::get_save_function(output_filepath);
      save_function(data.value(), output_filepath);
    }
  }

  return 0;
}