#pragma once
#include <stdexcept>
#include <string>
#include <string_view>
//
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace viewer {

class basic_socket {
 public:
  using handle_type = int;
  static constexpr size_t read_buffer_size = 256;

  basic_socket() : handle{socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0)} {
    if (!valid()) throw std::runtime_error("Failed to create socket.");
  }
  explicit basic_socket(handle_type h) noexcept : handle(h) {}

  virtual ~basic_socket() noexcept {
    if (valid()) close(handle);
  }

  basic_socket(const basic_socket&) = delete;
  basic_socket& operator=(const basic_socket&) = delete;

  basic_socket(basic_socket&& s) noexcept : handle{s.handle} { s.handle = -1; }
  basic_socket& operator=(basic_socket&& s) noexcept {
    std::swap(handle, s.handle);
    return *this;
  }

  bool valid() const noexcept { return handle != -1; }
  operator bool() const noexcept { return valid(); }

  void write(std::string_view str) {
    const auto size = ::write(handle, str.data(), str.size());
    if (size == -1)
      throw std::runtime_error(
          "Failed to write the given string to the socket.");
  }

  auto read() -> std::string {
    std::string result{};
    std::string buffer(read_buffer_size, '\0');
    int size = 0;
    do {
      size = ::read(handle, buffer.data(), buffer.size());
      if (size == -1) return result;
      result += std::string_view(buffer.data(), size);
    } while (size == buffer.size());
    return result;
  }

 protected:
  handle_type handle{};
};

class server_socket : public basic_socket {
 public:
  server_socket(std::string_view server_address)
      : basic_socket{}, name{server_address} {
    unlink(name.c_str());

    struct sockaddr_un address {
      .sun_family = AF_UNIX
    };
    strncpy(address.sun_path, name.c_str(), sizeof(address.sun_path) - 1);

    auto success = bind(handle, (const struct sockaddr*)&address,
                        sizeof(struct sockaddr_un));
    if (success == -1)
      throw std::runtime_error(
          "Failed to bind the server socket to the given address.");

    success = listen(handle, 20);
    if (success == -1)
      throw std::runtime_error("Failed to listen on the server socket.");
  }

  virtual ~server_socket() noexcept { unlink(name.c_str()); }

  auto accept() -> basic_socket {
    return basic_socket{::accept(handle, nullptr, nullptr)};
  }

 private:
  std::string name{};
};

class client_socket : public basic_socket {
 public:
  client_socket(std::string_view server_address)
      : basic_socket{}, server{server_address} {
    struct sockaddr_un address {
      .sun_family = AF_UNIX
    };
    strncpy(address.sun_path, server.c_str(), sizeof(address.sun_path) - 1);

    const auto success = connect(handle, (const struct sockaddr*)&address,
                                 sizeof(struct sockaddr_un));
    if (success == -1)
      throw std::runtime_error(
          "Failed to connect the client socket to the given server address.");
  }

 private:
  std::string server{};
};

}  // namespace viewer
