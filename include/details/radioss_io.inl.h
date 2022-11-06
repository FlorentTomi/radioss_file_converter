#include "radioss_io.h"

#include <type_traits>
#include <winsock2.h>
#include <boost/asio/detail/socket_ops.hpp>

namespace radioss::io::__details
{
template<std::size_t N>
struct swap_bytes_func
{
  template<class T>
  auto operator()(T&) const -> void
  {
  }
};

template<>
struct swap_bytes_func<2>
{
  using result_t = boost::asio::detail::u_short_type;

  template<class T>
  auto operator()(T& data) const -> void
  {
    auto& convData = reinterpret_cast<result_t&>(data);
    convData = boost::asio::detail::socket_ops::host_to_network_short(convData);
  }
};

template<>
struct swap_bytes_func<4>
{
  using result_t = boost::asio::detail::u_long_type;

  template<class T>
  auto operator()(T& data) const -> void
  {
    auto& convData = reinterpret_cast<result_t&>(data);
    convData = boost::asio::detail::socket_ops::network_to_host_long(convData);
  }
};

template<>
struct swap_bytes_func<8>
{
  using result_t = uint64_t;

  template<class T>
  auto operator()(T& data) const -> void
  {
    auto& convData = reinterpret_cast<result_t&>(data);
    convData = ::htonll(convData);
  }
};

template<class T>
static auto swap_bytes(T& data) -> void
{
  using swap_bytes_t = swap_bytes_func<sizeof(T)>;
  static constexpr swap_bytes_t SwapBytes;

  SwapBytes(data);
}

template<class T>
auto read_data(std::ifstream& stream, T& data) -> void;
auto read_data(std::ifstream& stream, std::size_t& data) -> void;

template<class T, std::size_t N, class... Args>
auto read_data(std::ifstream& stream, std::array<T, N>& data,
                      Args&&... args) -> void;

template<class T, class... Args>
auto read_data(std::ifstream& stream, std::vector<T>& data,
                      std::size_t size, Args&&... args) -> void;

auto read_data(std::ifstream& stream, std::string& data, std::size_t size)
    -> void;

auto read_data(std::ifstream& stream, radioss::Radioss::flags_t& flags) -> void;
} // namespace radioss::__details

template<class T>
static auto radioss::io::__details::read_data(std::ifstream& stream, T& data) -> void
{
  stream.read(reinterpret_cast<char*>(&data), sizeof(T));
  radioss::io::__details::swap_bytes(data);
}

template<class T, std::size_t N, class... Args>
static auto radioss::io::__details::read_data(std::ifstream& stream, std::array<T, N>& data,
                               Args&&... args) -> void
{
  for (T& v : data)
  {
    radioss::io::__details::read_data(stream, v, std::forward<Args>(args)...);
  }
}

template<class T, class... Args>
static auto radioss::io::__details::read_data(std::ifstream& stream, std::vector<T>& data,
                               std::size_t size, Args&&... args) -> void
{
  if (size == 0)
  {
    return;
  }
  
  data.resize(size);
  for (T& v : data)
  {
    radioss::io::__details::read_data(stream, v, std::forward<Args>(args)...);
  }
}