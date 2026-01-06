#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include <errno.h>
#include <iostream>

#include "config.h"
#include "physics.h"
#include "protocol.h"

namespace {

/*
 * Receives all data from the frontend socket connection to the buffer. Returns
 * false if the connection was closed or an error occurred.
 */
bool recv_all(int fd, void* buffer, size_t length) {
  uint8_t* ptr = static_cast<uint8_t*>(buffer);
  size_t remaining = length;
  while (remaining > 0) {
    ssize_t count = ::recv(fd, ptr, remaining, 0);
    if (count == 0) {
      return false;
    }
    if (count < 0) {
      if (errno == EINTR) {
        continue;
      }
      return false;
    }
    ptr += count;
    remaining -= static_cast<size_t>(count);
  }
  return true;
}

/*
 * Sends all data from the buffer to the frontend socket connection. Returns
 * false if the connection was closed or an error occurred.
 */
bool send_all(int fd, const void* buffer, size_t length) {
  const uint8_t* ptr = static_cast<const uint8_t*>(buffer);
  size_t remaining = length;
  while (remaining > 0) {
    ssize_t count = ::send(fd, ptr, remaining, 0);
    if (count <= 0) {
      if (count < 0 && errno == EINTR) {
        continue;
      }
      return false;
    }
    ptr += count;
    remaining -= static_cast<size_t>(count);
  }
  return true;
}

}

int main() {
  int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    std::cerr << "Failed to create socket\n";
    return 1;
  }

  int opt = 1;
  ::setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(config::kDefaultPort);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  if (::bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    std::cerr << "Bind failed\n";
    ::close(server_fd);
    return 1;
  }

  if (::listen(server_fd, 1) < 0) {
    std::cerr << "Listen failed\n";
    ::close(server_fd);
    return 1;
  }

  std::cout << "Waiting for client on 127.0.0.1:" << config::kDefaultPort << "...\n";
  int client_fd = ::accept(server_fd, nullptr, nullptr);
  if (client_fd < 0) {
    std::cerr << "Accept failed\n";
    ::close(server_fd);
    return 1;
  }

  ::setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));

  protocol::InitMsg init{};
  if (!recv_all(client_fd, &init, sizeof(init))) {
    std::cerr << "Failed to read init message\n";
    ::close(client_fd);
    ::close(server_fd);
    return 1;
  }

  if (init.magic != config::kInitMagic) {
    std::cerr << "Bad init magic\n";
    ::close(client_fd);
    ::close(server_fd);
    return 1;
  }

  const float width = static_cast<float>(init.width);
  const float height = static_cast<float>(init.height);
  const int num_particles = static_cast<int>(init.num_particles);
  if (num_particles <= 0) {
    std::cerr << "Invalid particle count\n";
    ::close(client_fd);
    ::close(server_fd);
    return 1;
  }

  Simulation simulation(num_particles, width, height);

  protocol::InputFrame input{};
  while (true) {
    if (!recv_all(client_fd, &input, sizeof(input))) {
      std::cerr << "Client disconnected\n";
      break;
    }

    simulation.Step(input);

    const auto& buffer = simulation.RenderBuffer();
    if (!send_all(client_fd, buffer.data(), buffer.size() * sizeof(float))) {
      std::cerr << "Failed to send frame\n";
      break;
    }
  }

  ::close(client_fd);
  ::close(server_fd);
  return 0;
}
