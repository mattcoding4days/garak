//
// Created by matt on 9/5/22.
//
#include <garak/net/server.hpp>

namespace garak::net {
Server::Server(garak::u16 port)
    : mPort(port), mAsioAcceptor(mAsioContext, asio::ip::tcp::v4(), port) {}

Server::~Server() { this->stop(); }

bool Server::startAcceptingConnections() noexcept {
  try {
    // NOTE: the order here matters, we need to issue some work to the asio
    // context, so it doesn't immediately shut down, so we instruct the server
    // to wait for a client connection, while we run the context in a separate
    // thread.
    this->waitForConnection();
  } catch (const std::exception &error) {
    std::cerr << "Server Error: " << error.what() << '\n';

    return false;
  }
  return true;
}

void Server::run() noexcept {
  mThreadContext = std::thread([this]() { mAsioContext.run(); });
}

void Server::stop() {
  if (!mAsioContext.stopped()) {
    mAsioContext.stop();
  }
  if (mThreadContext.joinable()) {
    mThreadContext.join();
  }
  std::cout << "Server is shutting down" << '\n';
}

void Server::waitForConnection() {
  mAsioAcceptor.async_accept([this](std::error_code error_code,
                                    asio::ip::tcp::socket socket) {
    if (!error_code) {
      ConnectionMetaData metaData(
          socket.remote_endpoint().address().to_string(),
          socket.remote_endpoint().port());

      std::cerr << "Server: New connection: " << metaData.basicInfo() << '\n';

      auto new_connection = std::make_shared<Connection>(
          metaData, Connection::Owner::server, mAsioContext, std::move(socket),
          mQueMessagesIn);

      if (this->onClientConnect(new_connection)) {
        // add the new connection to our client container
        mClientConnections.emplace_back(std::move(new_connection));

        // establish connection and assign the client connection an id
        mClientConnections.back()->connectToClient(++n_id_counter);

      } else {
        std::cout << "Server: Connection denied: " << metaData.basicInfo() << '\n';
      }
    } else {
      // regardless if the connection was approved or denied,
      // we need to re-prime the context with work, so it doesn't
      // shut down.
      std::cerr << "Server Error: " << error_code.message() << '\n';
    }
    this->waitForConnection();
  });
}

void Server::notifyClient(std::shared_ptr<Connection> &client,
                              const Message &msg) {
  // first we must verify that the client is actually connected.
  // One of the limitations of tcp, is that we are not told when
  // a client disconnects, we only find out when we try to talk to said
  // client.
  if (client && client->isConnected()) {
    client->send(msg);
  } else {
    onClientDisconnect(client);
    // reset the smart pointer, so it is nullptr
    client.reset();
    // remove the actual connection from the deque.A
    std::erase(mClientConnections, client);
  }
}

void Server::notifyAllClients(Message const &msg,
                      std::shared_ptr<Connection> ignoreClient) {
}

}  // namespace garak::net