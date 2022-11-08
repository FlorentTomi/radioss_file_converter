
#include "radioss.h"
#include "radioss_io.h"

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

static auto get_save_function(std::filesystem::path const& filepath)
    -> std::function<void(radioss::Radioss const&,
                          std::filesystem::path const&)>
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

auto main(int argc, char** argv) -> int
{
  namespace boost_po = boost::program_options;

  static constexpr std::string_view ExampleIn = "../example_files/Ellipsoid_75_1_1_300_105A001";
  static constexpr std::string_view ExampleOut = "test.csv";

  auto opt_description = boost_po::options_description{"allowed options"};

  // clang-format off
  opt_description.add_options()
    ("help,h", "produce help message")
    ("input,i", boost_po::value<std::filesystem::path>()->required()->default_value(ExampleIn.data()), "input file (RunnameAXXX)")
    ("output,o", boost_po::value<std::filesystem::path>()->required()->default_value(ExampleOut.data()), "output file (*.json, *.vtk, *.csv)");
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

  auto input_filepath = var_map.at("input").as<std::filesystem::path>();
  if (!::validate_file(input_filepath))
  {
    std::cout << "invalid file: " << input_filepath << std::endl;
    return -1;
  }

  std::cout << "\t- converting file: " << input_filepath << std::endl;

  auto output_filepath = var_map.at("output").as<std::filesystem::path>();
  auto save_function = ::get_save_function(output_filepath);
  if (!save_function)
  {
    std::cout << "invalid output extension" << std::endl;
    return -1;
  }

  auto data = radioss::io::read(input_filepath);
  if (!data.has_value())
  {
    std::cout << "invalid Radioss file: " << input_filepath << std::endl;
    return -1;
  }

  save_function(data.value(), output_filepath);

  return 0;
}