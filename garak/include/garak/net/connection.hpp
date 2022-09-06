// Created by matt on 8/31/22. The following code is based (however garak
// is heavily modified) on Javidx9's Tcp networking tutorial on YouTube.
// below is his license. Thanks Javid

/*
        Copyright 2018 - 2020 OneLoneCoder.com
        Redistribution and use in source and binary forms, with or without
        modification, are permitted provided that the following conditions
        are met:
        1. Redistributions or derivations of source code must retain the above
        copyright notice, this list of conditions and the following disclaimer.
        2. Redistributions or derivative works in binary form must reproduce
        the above copyright notice. This list of conditions and the following
        disclaimer must be reproduced in the documentation and/or other
        materials provided with the distribution.
        3. Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived
        from this software without specific prior written permission.
        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
        "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
        LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
        A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
        HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
        SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
        LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
        DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
        THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
        (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
        OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LIBGARAK_NET_CONNECTION_HPP
#define LIBGARAK_NET_CONNECTION_HPP

#include <asio.hpp>
#include <garak/containers/module.hpp>
#include <garak/net/error.hpp>
#include <garak/net/message.hpp>
#include <garak/utils/module.hpp>

namespace garak::net {
/**
 * @brief A container for meta data about a connection.
 *
 * @details In
 * */
class ConnectionMetaData {
 public:
  /**
   * @brief Descriptive object to use as a parameter when updating
   * ConnectionMetaData' internal map with data
   * */
  struct AdditionalInfo {
    std::string mKey{};
    std::string mValue{};
  };

 public:
  ConnectionMetaData() = default;
  [[maybe_unused]] ConnectionMetaData(std::string ipaddress, u16 port);

  virtual ~ConnectionMetaData() = default;
  ConnectionMetaData(ConnectionMetaData const &) = default;
  ConnectionMetaData &operator=(ConnectionMetaData const &) = default;
  ConnectionMetaData(ConnectionMetaData &&) noexcept = default;
  ConnectionMetaData &operator=(ConnectionMetaData &&) noexcept = default;

 public:
  /**
   * @brief Getter for port
   * */
  [[nodiscard]] u16 getPort() const noexcept;

  /**
   * @brief Getter for ip4 address
   * */
  [[nodiscard]] std::string getIpAddress() const noexcept;

  /**
   * @brief Concatenates the port on the end of the ip4 address, seperated
   * by a colon
   * */
  [[nodiscard]] std::string basicInfo() const noexcept;

  /**
   * @brief Get a value out of the internal map by key
   *
   * @param key refers to the key in the hashmap
   *
   * @details Exception safe, if the internal map does not contain the key,
   * the optional value will contain std::nullopt.
   *
   * @returns std::optional<std::string>
   * */
  [[nodiscard]] std::optional<std::string> getAdditionalInfoByKey(
      std::string const &key) const noexcept;

  /**
   * @brief Update/setter for the underlying map<std::string, std::string>,
   * uses move semantics.
   * */
  [[maybe_unused]] void updateAdditionalInfo(ConnectionMetaData::AdditionalInfo &info) noexcept;

  /**
   * @brief Update/setter for the underlying map<std::string, std::string> for
   * move semantics with inplace object construction.
   * */
  [[maybe_unused]] void updateAdditionalInfo(ConnectionMetaData::AdditionalInfo &&info) noexcept;

 private:
  std::string pIpaddress{};
  u16 pPort{};
  std::map<std::string, std::string> pAdditionalInfo{};
};

/**
 * @brief Wraps the asio::tcp::socket
 *
 * @details The connection is responsible for reading and writing data to the
 * socket. It is also responsible for managing the lifetime of the socket.
 * */
class Connection : public std::enable_shared_from_this<Connection> {
 public:
  /**
   * @brief Defines which implementation owns this connection
   * */
  enum class Owner {
    server,
    client,
  };

 public:
  /**
   * @brief The only constructor that should be used to properly set,
   *  up a connection for a server, or a client
   *
   * @param metaData a shared_ptr to the meta data interface
   * @param parent sets the owner of this particular connection
   * @param asio_context is needed for the connection to be established
   * @param socket which will be used in tandem with the context
   * @param incomingQueue each connection will have a queue of incoming
   * messages
   * */
  Connection(ConnectionMetaData metaData, Owner parent,
             asio::io_context &asioContext, asio::ip::tcp::socket socket,
             containers::ThreadSafeDeque<OwnedMessage> &incomingQueue);

  virtual ~Connection() = default;

  /**
   * @brief Because asios socket and context objects have deleted
   * copy constructors and move constructors, we must do the same
   * */
  Connection(Connection const &) = delete;
  Connection &operator=(Connection const &) = delete;
  Connection(Connection &&) noexcept = delete;
  Connection &operator=(Connection &&) noexcept = delete;

 public:
  /**
   * @brief Get a copy to the this connections Ipv4 address
   *
   * @details Convenience method for the underlying ConnectionMetaData object
   * */
  [[nodiscard]] std::string getIp() const noexcept;

  /**
   * @brief Get a copy of the connections port
   *
   * @details Convenience method for the underlying ConnectionMetaData object
   * */
  [[nodiscard]] u16 getPort() const noexcept;

  /**
   * @brief Get a mutable reference to the underlying ConnectionMetaData object
   *
   * @details Convenience method for the underlying ConnectionMetaData object
   * */
  [[nodiscard]] ConnectionMetaData &getConnectionMetaData() noexcept;

  /**
   * @brief If the owner of this connection is the server, then connect to
   * said client, and begin reading the incoming socket stream
   * */
  [[maybe_unused]] void connectToClient() noexcept;

  /**
   * @brief Async: Connect to the server
   *
   * @details Wraps asio's async_connect method
   *
   * @param endpoint The endpoints ipaddress and port
   *
   * @throws ConnectionError if connection to the endpoint cannot be acquired
   * */
  [[maybe_unused]] void connectToServer(
      asio::ip::tcp::resolver::results_type const &endpoint);

  /**
   * @brief if the connection is active, close the socket
   * */
  void disconnect();

  /**
   * @brief Tests if the underlying socket is connected
   * */
  [[nodiscard]] bool isConnected() const noexcept;

  /**
   * @brief Async - send a message, connections are one-to-one, so no need to
   * specify the target, for a client, the target is the server and vice versa
   **/
  void send(Message const &msg);

 private:
  /**
   * @brief Async: Wraps asio::async_write
   *
   * @details Prime the context, ready to write a message body,
   * */
  void writeStream_();

  /**
   * @brief Async: Prime the context ready to read a message body
   * */
  void readStream_();

  /**
   * @brief If the owner of this connection is a server, we need to create an
   *  owned message,
   * */
  void addToIncomingMessageQueue_();

 private:
  /**
   * @brief Metadata for this connection
   * */
  ConnectionMetaData pMetaData{};

  /**
   * @brief Default owner is server
   * */
  Owner pOwnerType = Owner::server;

  /**
   * @brief a Socket can't function without an io context, a server can
   * have multiple connections, but we can't have multiple contexts,
   * the context should behave in tandem, so the connection needs to be
   * provided a context by the server/client interface
   * */
  asio::io_context &pAsioContext;

  /**
   * @brief Each connection has a unique socket to a remote
   * */
  asio::ip::tcp::socket pSocket;

  /**
   * @brief The read buffer
   * */
  asio::streambuf pReadBuffer{};

  /**
   * @brief This deque holds all messages that have been received from
   * the remote side of this connection. Note it is a reference,
   * as the "owner" of this connection is expected to provide a queue,
   * great care must be taken here so that the queue does not become a dead
   * reference
   * */
  containers::ThreadSafeDeque<OwnedMessage> &pQmessagesIn;

  /**
   * @brief This deque holds all messages to be sent to the remote side
   *  of this connection
   * */
  containers::ThreadSafeDeque<Message> pQmessagesOut{};

  /**
   * @brief a temporary holder for the current message to be passed to the
   *  asio context
   * */
  Message pTempMsg{};
};
}  // namespace garak::net

#endif  // LIBGARAK_NET_CONNECTION_HPP
