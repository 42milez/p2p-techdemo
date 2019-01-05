#include <iostream>

#include "engine/network/NetworkShared.h"
#include "engine/network/SocketAddress.h"
#include "engine/network/SocketAddressFactory.h"
#include "engine/network/SocketUtil.h"

#include "Network.h"

namespace auth_server {
  namespace {
    using KEVENT_REGISTER_STATUS = engine::network::KEVENT_REGISTER_STATUS;
    using SocketAddress = engine::network::SocketAddress;
    using SocketUtil = engine::network::SocketUtil;

    const uint32_t SERVER_ADDRESS = INADDR_ANY;
    const uint16_t SERVER_PORT = 12345;
  }

  Network::Network() {
    logger_ = spdlog::basic_logger_mt("auth_server / Network", "logs/development.log");
  }

  bool Network::init() {
    server_socket_ = engine::network::SocketUtil::create_tcp_socket(engine::network::SocketAddressFamily::INET);

    if (server_socket_ == nullptr) {
      return false;
    }

    SocketAddress server_address(SERVER_ADDRESS, SERVER_PORT);

    // TODO: Retry when bind() fails
    server_socket_->bind(server_address);
    server_socket_->listen(5);
    mux_ = SocketUtil::create_multiplexer();

    if (SocketUtil::register_event(mux_, server_socket_) == KEVENT_REGISTER_STATUS::FAIL) {
      return false;
    }

    return true;
  }

  void Network::process_incoming_packets() {
    // MEMO: Don't initialize events_ as the nfds indicates the number of the events.
    auto nfds = check_kernel_event(events_);

    if (nfds == -1) {
      // error
      // TODO: Output logs
      return;
    } else if (nfds == 0) {
      // timeout
      // TODO: Output logs
      return;
    } else {
      // TODO: Use queue
      std::vector<TCPSocketPtr> ready_sockets;

      for (auto i = 0; i < nfds; i++) {
        auto soc = (int) events_[i].ident;

        if (server_socket_->is_same_descriptor(soc)) {
          accept_incoming_packets();
        } else {
          ready_sockets.push_back(client_sockets_.at(soc));
        }
      }

      read_incoming_packets_into_queue(ready_sockets);
    }
  }

  int Network::check_kernel_event(struct kevent events[]) {
    /*
      The kevent(), kevent64() and kevent_qos() system calls return the number of events placed in the eventlist, up to
      the value given by nevents.  If an error occurs while processing an element of the changelist and there is enough
      room in the eventlist, then the event will be placed in the eventlist with EV_ERROR set in flags and the system
      error in data.  Otherwise, -1 will be returned, and errno will be set to indicate the error condition.  If the
      time limit expires, then kevent(), kevent64() and kevent_qos() return 0.
    */
    return kevent(mux_, nullptr, 0, events, N_KEVENTS, nullptr);
  }

  void Network::accept_incoming_packets() {
    auto tcp_socket = server_socket_->accept();
    store_client(tcp_socket);
    SocketUtil::register_event(mux_, tcp_socket);
  }

  void Network::read_incoming_packets_into_queue(const std::vector<TCPSocketPtr> &ready_sockets) {
    char buffer[1500];
    memset(buffer, 0, 1500);

    for (const auto &socket : ready_sockets) {
      auto read_byte_count = socket->recv(buffer, sizeof(buffer));

      if (read_byte_count == 0) {
        // For TCP sockets, the return value 0 means the peer has closed its half side of the connection.
        // TODO
        //HandleConnectionReset( fromAddress );
        delete_client(socket->descriptor());
      } else if(read_byte_count == -engine::network::WSAECONNRESET) {
        // TODO
        //HandleConnectionReset( fromAddress );
        delete_client(socket->descriptor());
      } else if (read_byte_count > 0) {
        buffer[read_byte_count] = '\0';
        std::cout << buffer << std::endl;
      }
      else {
        // uhoh, error? exit or just keep going?
      }
    }
  }

  void Network::store_client(const TCPSocketPtr &tcp_socket) {
    client_sockets_.insert(std::make_pair(tcp_socket->descriptor(), tcp_socket));
  }

  void Network::delete_client(int key) {
    client_sockets_.erase(key);
  }
} // namespace auth_server
