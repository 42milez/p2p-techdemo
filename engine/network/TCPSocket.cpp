#include <unistd.h>

#include <sys/socket.h>

#include "SocketUtil.h"

namespace engine
{
  namespace network
  {
    using SocketUtil = engine::network::SocketUtil;

    TCPSocket::~TCPSocket() {
      close(socket_);
    }

    int TCPSocket::bind(const SocketAddress &address) {
      int error = ::bind(socket_, &address.sockaddr_, address.size());
      if (error != 0) {
        SocketUtil::report_error("TCPSocket::bind");
        return SocketUtil::last_error();
      }

      return NO_ERROR;
    }

    int TCPSocket::connect(const SocketAddress &address) {
      int err = ::connect(socket_, &address.sockaddr_, address.size());
      if (err < 0) {
        SocketUtil::report_error("TCPSocket::Connect");
        return SocketUtil::last_error();
      }
      return NO_ERROR;
    }

    ssize_t TCPSocket::send(const void *data, size_t len) {
      ssize_t bytes_sent = ::send(socket_, static_cast<const char *>(data), len, 0);
      if (bytes_sent < 0) {
        SocketUtil::report_error("TCPSocket::send");
        return SocketUtil::last_error();
      }
      // TODO using suitable data type for platform
      return bytes_sent;
    }

    ssize_t TCPSocket::recv(void *data, size_t len) {
      ssize_t bytes_received_count = ::recv(socket_, static_cast<char *>(data), len, 0);
      if (bytes_received_count < 0) {
        SocketUtil::report_error("TCPSocket::Receive");
        return -SocketUtil::last_error();
      }
      return bytes_received_count;
    }

    int TCPSocket::listen(int backlog) {
      int err = ::listen(socket_, backlog);
      if (err < 0) {
        SocketUtil::report_error("TCPSocket::Listen");
        return -SocketUtil::last_error();
      }
      return NO_ERROR;
    }

    TCPSocketPtr TCPSocket::accept(SocketAddress &address) {
      socklen_t len = address.size();
      Socket socket = ::accept(socket_, &address.sockaddr_, &len);

      if (socket != INVALID_SOCKET) {
        return TCPSocketPtr(new TCPSocket(socket));
      } else {
        SocketUtil::report_error("TCPSocket::Accept");
        return nullptr;
      }
    }
  }
}