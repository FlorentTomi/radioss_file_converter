#include "radioss.h"
#include "io/property_tree.h"
#include "radioss_element.h"
#include "radioss_geometry.h"

#include <type_traits>
#include <vtkCellArray.h>
#include <vtkCellType.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>

auto radioss::Radioss::to_tree() const -> boost::property_tree::ptree
{
  boost::property_tree::ptree ptree;
  if (!valid())
  {
    return ptree;
  }

  ptree.put("file_type", file_time);
  ptree.put("time", time);
  ptree.put("mod_anim", mod_anim);
  ptree.put("run", run);

  geometry_2d.add_to_ptree(ptree, "geometry_2d");

  if (geometry_3d.has_value())
  {
    geometry_3d->add_to_ptree(ptree, "geometry_3d");
  }

  if (geometry_1d.has_value())
  {
    geometry_1d->add_to_ptree(ptree, "geometry_1d");
  }

  if (time_history.has_value())
  {
    time_history->add_to_ptree(ptree, "time_history");
  }

  if (geometry_sph.has_value())
  {
    geometry_sph->add_to_ptree(ptree, "geometry_sph");
  }

  boost::property_tree::ptree subset_ptree;
  for (auto const& subset : subsets)
  {
    boost::property_tree::ptree subset_child;
    subset.add_to_ptree(subset_child, "subset");
    subset_ptree.push_back(std::make_pair("", subset_child));
  }
  ptree.put_child("subsets", subset_ptree);

  ::io::add_to_ptree(ptree, "materials", materials);
  ::io::add_to_ptree(ptree, "properties", properties);

  return ptree;
}

auto radioss::Radioss::to_unstructured_grid() const
    -> vtkSmartPointer<vtkUnstructuredGrid>
{
  if (!valid())
  {
    return nullptr;
  }

  auto unstructured_grid = vtkSmartPointer<vtkUnstructuredGrid>::New();

  {
    auto field_data = vtkNew<vtkFieldData>();

    auto time_data = vtkNew<vtkFloatArray>();
    time_data->SetName("TIME");
    time_data->SetNumberOfValues(1);
    time_data->SetValue(0, file_time);
    field_data->AddArray(time_data);

    auto cycle_data = vtkNew<vtkIntArray>();
    cycle_data->SetName("CYCLE");
    cycle_data->SetNumberOfValues(1);
    cycle_data->SetValue(0, 0);
    field_data->AddArray(cycle_data);

    unstructured_grid->SetFieldData(field_data);
  }

  geometry_2d.add_to_unstructured_grid(*unstructured_grid);
  
  if (geometry_1d.has_value())
  {
    geometry_1d->add_to_unstructured_grid(*unstructured_grid);
  }
  
  if (geometry_3d.has_value())
  {
    geometry_3d->add_to_unstructured_grid(*unstructured_grid);
  }
  
  if (geometry_sph.has_value())
  {
    geometry_sph->add_to_unstructured_grid(*unstructured_grid);
  }

  {
    auto cells = vtkNew<vtkCellArray>();

    std::vector<std::underlying_type_t<VTKCellType>> cell_types;
    cell_types.reserve(
        geometry_2d.elements.size() +
        (geometry_1d.has_value() ? geometry_1d->elements.size() : 0) +
        (geometry_3d.has_value() ? geometry_3d->elements.size() : 0) +
        (geometry_sph.has_value() ? geometry_sph->elements.size() : 0));

    if (geometry_1d.has_value())
    {
      for (auto const& element : geometry_1d->elements)
      {
        cells->InsertNextCell(
            std::tuple_size_v<radioss::Element1D::connectivity_t>);
        cell_types.emplace_back(VTK_LINE);
        for (auto ptId : element.connectivity)
        {
          cells->InsertCellPoint(ptId);
        }
      }
    }

    for (auto const& element : geometry_2d.elements)
    {
      cells->InsertNextCell(
          std::tuple_size_v<radioss::Element2D::connectivity_t>);
      cell_types.emplace_back(VTK_QUAD);
      for (auto ptId : element.connectivity)
      {
        cells->InsertCellPoint(ptId);
      }
    }

    if (geometry_3d.has_value())
    {
      for (auto const& element : geometry_3d->elements)
      {
        cells->InsertNextCell(
            std::tuple_size_v<radioss::Element3D::connectivity_t>);
        cell_types.emplace_back(VTK_HEXAHEDRON);
        for (auto ptId : element.connectivity)
        {
          cells->InsertCellPoint(ptId);
        }
      }
    }

    if (geometry_sph.has_value())
    {
      for (auto const& element : geometry_sph->elements)
      {
        cells->InsertNextCell(1);
        cell_types.emplace_back(VTK_VERTEX);
        cells->InsertCellPoint(element.connectivity);
      }
    }

    unstructured_grid->SetCells(cell_types.data(), cells);
  }

  return unstructured_grid;
}

auto radioss::TimeHistory::Token::add_to_ptree(
    boost::property_tree::ptree& ptree, std::string const& key) const -> void
{
  boost::property_tree::ptree child;
  child.put("name", name);
  child.put("internal_number", internal_number);
  ptree.put_child(key, child);
}

auto radioss::TimeHistory::add_to_ptree(boost::property_tree::ptree& ptree,
                                        std::string const& key) const -> void
{
  boost::property_tree::ptree child;

  boost::property_tree::ptree node_ptree;
  for (auto const& token : nodes)
  {
    boost::property_tree::ptree node_child;
    token.add_to_ptree(node_child, "node");
    node_ptree.push_back(std::make_pair("", node_child));
  }
  child.put_child("nodes", node_ptree);

  boost::property_tree::ptree element_2d_ptree;
  for (auto const& token : elements_2d)
  {
    boost::property_tree::ptree element_child;
    token.add_to_ptree(element_child, "element_2d");
    element_2d_ptree.push_back(std::make_pair("", element_child));
  }
  child.put_child("elements_2d", element_2d_ptree);

  boost::property_tree::ptree element_3d_ptree;
  for (auto const& token : elements_3d)
  {
    boost::property_tree::ptree element_child;
    token.add_to_ptree(element_child, "element_3d");
    element_3d_ptree.push_back(std::make_pair("", element_child));
  }
  child.put_child("elements_3d", element_3d_ptree);

  boost::property_tree::ptree element_1d_ptree;
  for (auto const& token : elements_1d)
  {
    boost::property_tree::ptree element_child;
    token.add_to_ptree(element_child, "element_1d");
    element_1d_ptree.push_back(std::make_pair("", element_child));
  }
  child.put_child("elements_1d", element_1d_ptree);

  ptree.put_child(key, child);
}

auto radioss::Subset::add_to_ptree(boost::property_tree::ptree& ptree,
                                   std::string const& key) const -> void
{
  boost::property_tree::ptree child;

  child.put("name", name);
  child.put("parent_number", parent_number);
  child.put("subset_sons", subset_sons);
  child.put("subpart_2d", subpart_2d);
  child.put("subpart_3d", subpart_3d);
  child.put("subpart_1d", subset_sons);

  ptree.put_child(key, child);
}