#include "radioss_geometry.h"
#include "io/utils.h"

#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkCellData.h>

#include <algorithm>
#include <sstream>

namespace radioss::io::__details
{
auto read_data(std::ifstream& stream, radioss::GeometrySPH& geometry,
               std::bitset<10> const& flags) -> void;
auto read_data(std::ifstream& stream, radioss::Geometry1D& geometry,
               std::bitset<10> const& flags) -> void;
auto read_data(std::ifstream& stream, radioss::Geometry2D& geometry,
               std::bitset<10> const& flags) -> void;
auto read_data(std::ifstream& stream, radioss::Geometry3D& geometry,
               std::bitset<10> const& flags) -> void;
} // namespace radioss::io::__details

template<std::size_t Dimension, class Element>
auto radioss::Geometry<Dimension, Element>::add_to_ptree(
    boost::property_tree::ptree& ptree, std::string const& key) const -> void
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

  ptree.put_child(key, child);
}

template<std::size_t Dimension, class Element>
auto radioss::Geometry<Dimension, Element>::add_to_unstructured_grid(
    vtkUnstructuredGrid& unstructured_grid) const -> void
{
  auto cell_data = unstructured_grid.GetCellData();

  auto element_numbers = cell_data->GetArray("ELEMENT_ID");
  if (element_numbers == nullptr)
  {
    auto new_element_numbers = vtkNew<vtkIntArray>();
    new_element_numbers->SetName("ELEMENT_ID");
    new_element_numbers->SetNumberOfComponents(1);

    cell_data->AddArray(new_element_numbers);
    element_numbers = new_element_numbers;
  }

  auto erosion_status = cell_data->GetArray("EROSION_STATUS");
  if (erosion_status == nullptr)
  {
    auto new_erosion_status = vtkNew<vtkIntArray>();
    new_erosion_status->SetName("EROSION_STATUS");
    new_erosion_status->SetNumberOfComponents(1);

    cell_data->AddArray(new_erosion_status);
    erosion_status = new_erosion_status;
  }

  std::string element_prefix;
  {
    std::ostringstream str;
    str << dimension() << "DELEM_";
    element_prefix = str.str();
  }

  for (auto const& element : elements)
  {
    static_cast<vtkIntArray*>(element_numbers)
        ->InsertNextValue(element.internal_number.value_or(-1));
    static_cast<vtkIntArray*>(erosion_status)
        ->InsertNextValue(element.deleted ? 1 : 0);

    for (auto const& [scalar_function_name, scalar_function] :
         element.scalar_functions)
    {
      auto fixed_scalar_function_name =
          element_prefix + radioss::io::utils::fix_name(scalar_function_name);
      auto* scalar_function_data =
          cell_data->GetArray(fixed_scalar_function_name.c_str());
      if (scalar_function_data == nullptr)
      {
        auto new_scalar_function_data = vtkNew<vtkFloatArray>();
        new_scalar_function_data->SetName(fixed_scalar_function_name.c_str());
        new_scalar_function_data->SetNumberOfComponents(1);

        cell_data->AddArray(new_scalar_function_data);
        scalar_function_data = new_scalar_function_data;
      }

      static_cast<vtkFloatArray*>(scalar_function_data)
          ->InsertNextValue(scalar_function);
    }

    for (auto const& [tensor_name, tensor] : element.tensors)
    {
      auto fixed_tensor_name = element_prefix + radioss::io::utils::fix_name(tensor_name);
      auto* tensor_data = cell_data->GetArray(fixed_tensor_name.c_str());
      if (tensor_data == nullptr)
      {
        auto new_tensor_data = vtkNew<vtkFloatArray>();
        new_tensor_data->SetName(fixed_tensor_name.c_str());
        new_tensor_data->SetNumberOfComponents(element.tensor_size());

        cell_data->AddArray(new_tensor_data);
        tensor_data = new_tensor_data;
      }

      for (auto const& tensor_v : tensor)
      {
        static_cast<vtkFloatArray*>(tensor_data)->InsertNextValue(tensor_v);
      }
    }
  }

  // TODO PART_ID
}