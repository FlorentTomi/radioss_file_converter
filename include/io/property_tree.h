#ifndef _PROPERTY_TREE_H_
#define _PROPERTY_TREE_H_

#include <boost/property_tree/ptree.hpp>

#include <array>
#include <map>
#include <sstream>
#include <string>
#include <type_traits>

namespace io
{
template<class T, std::size_t N>
struct array_translator
{
  using internal_type = std::string;
  using external_type = std::array<T, N>;

  auto get_value(internal_type const& str) -> boost::optional<external_type>
  {
    if (!str.empty())
    {
      internal_type val;
      std::stringstream s(str);
      for (std::size_t i = 0; i < N; ++i)
      {
        s >> val[i];
      }
      return boost::optional<external_type>(val);
    }
    else
    {
      return boost::optional<external_type>(boost::none);
    }
  }

  auto put_value(external_type const& b) -> boost::optional<internal_type>
  {
    std::stringstream ss;
    ss << b[0];
    for (std::size_t i = 1; i < N; ++i)
    {
      ss << " " << b[i];
    }
    
    return boost::optional<internal_type>(ss.str());
  }
};

template<class T>
struct vector_translator
{
  using internal_type = std::string;
  using external_type = std::vector<T>;

  auto get_value(internal_type const& str) -> boost::optional<external_type>
  {
    if (!str.empty())
    {
      internal_type val;
      std::stringstream s(str);
      while (s)
      {
        s >> val.emplace_back();
      }
      return boost::optional<external_type>(val);
    }
    else
    {
      return boost::optional<external_type>(boost::none);
    }
  }

  auto put_value(external_type const& b) -> boost::optional<internal_type>
  {
    if (b.empty())
    {
      return boost::optional<internal_type>("");
    }

    std::stringstream ss;
    ss << b[0];
    for (std::size_t i = 1; i < b.size(); ++i)
    {
      ss << " " << b[i];
    }

    return boost::optional<internal_type>(ss.str());
  }
};

template<class T>
auto add_to_ptree(boost::property_tree::ptree& ptree, std::string const& key,
                  T const& data) -> void;

template<class T0, class T1>
auto add_to_ptree(boost::property_tree::ptree& ptree, std::string const& key,
                  std::map<T0, T1> const& data) -> void;
} // namespace io

template<class T>
auto io::add_to_ptree(boost::property_tree::ptree& ptree,
                      std::string const& key, T const& data) -> void
{
  ptree.put(key, data);
}

template<class T0, class T1>
auto io::add_to_ptree(boost::property_tree::ptree& ptree,
                      std::string const& key, std::map<T0, T1> const& data)
    -> void
{
  boost::property_tree::ptree child;
  for (auto const& [k, v] : data)
  {
    io::add_to_ptree(child, k, v);
  }
  ptree.put_child(key, child);
}

namespace boost::property_tree
{
template<typename Ch, typename Traits, typename Alloc, class T, std::size_t N>
struct translator_between<std::basic_string<Ch, Traits, Alloc>,
                          std::array<T, N>>
{
  using type = ::io::array_translator<T, N>;
};

template<typename Ch, typename Traits, typename Alloc, class T>
struct translator_between<std::basic_string<Ch, Traits, Alloc>, std::vector<T>>
{
  using type = ::io::vector_translator<T>;
};
} // namespace boost::property_tree

#endif // _PROPERTY_TREE_H_