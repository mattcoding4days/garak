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
#include <garak/net/interfaces.hpp>
#include <garak/net/message.hpp>
#include <garak/utils/module.hpp>

namespace garak::net {

/**
 * @brief Wraps the asio::tcp::socket
 *
 * @details The connection is responsible for reading and writing data to the
 * socket. It is also responsible for managing the lifetime of the socket.
 * */
template <class T>
class Connection : public std::enable_shared_from_this<Connection<T>> {
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
   * messages, and outgoing messages.
   * */
  Connection(ConnectionMetaDataPtr metaData, Owner parent,
             asio::io_context &asioContext, asio::ip::tcp::socket socket,
             containers::ThreadSafeDeque<OwnedMessage<T>> &incomingQueue)
      : pMetaData(std::move(metaData)),
        pOwnerType(parent),
        pAsioContext(asioContext),
        pSocket(std::move(socket)),
        pQmessagesIn(incomingQueue) {}

  virtual ~Connection() = default;
  Connection(Connection const &) = default;
  Connection &operator=(Connection const &) = default;
  Connection(Connection &&) noexcept = default;
  Connection &operator=(Connection &&) noexcept = default;

 public:
  /**
   * @brief Get a copy to the this connections Ipv4 address
   *
   * @details Convenience method, the user could just use getMetaData
   * */
  [[nodiscard]] std::string getIp() const noexcept {
    return pMetaData->mIpaddress;
  }

  /**
   * @brief Get a copy of the connections port
   *
   * @details Convenience method, the user could just use getMetaData
   * */
  [[nodiscard]] u8 getPort() const noexcept { return pMetaData->mPort; }

  /**
   * @brief Get a pointer of the connections MetaData
   *
   * @details This will only be of a major use for server code.
   *
   * @return std::shared_ptr<IConnectionMetaData>
   * */
  [[nodiscard]] ConnectionMetaDataPtr getMetaData() const noexcept {
    return pMetaData;
  }

  /**
   * @brief If the owner of this connection is the server, then connect to
   * said client, and begin reading the incoming socket stream
   */
  [[maybe_unused]] void connectToClient() noexcept {
    if (pOwnerType == Owner::server) {
      if (pSocket.is_open()) {
        pMetaData->mPort = pSocket.remote_endpoint().port();
        pMetaData->mIpaddress = pSocket.remote_endpoint().address().to_string();
        this->readStream_();
      }
    }
  }

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
      asio::ip::tcp::resolver::results_type const &endpoint) {
    if (pOwnerType == Owner::client) {
      // request asio attempts to connect to an endpoint
      asio::async_connect(
          pSocket, endpoint,
          [this](std::error_code error_code,
                 asio::ip::tcp::endpoint const & /*endpoint*/) {
            if (!error_code) {
              pMetaData->mPort = pSocket.remote_endpoint().port();
              pMetaData->mIpaddress =
                  pSocket.remote_endpoint().address().to_string();
              this->readStream_();
            } else {
              throw ConnectionError(error_code.message(), error::RUNTIME_INFO);
            }
          });
    }
  }

  /**
   * @brief if the connection is active, close the socket
   * */
  void disconnect() {
    if (this->isConnected()) {
      asio::post(pAsioContext, [this]() { pSocket.close(); });
    }
  }

  /**
   * @brief Tests if the underlying socket is connected
   * */
  [[nodiscard]] bool isConnected() const noexcept { return pSocket.is_open(); }

  /**
   * @brief Async - send a message, connections are one-to-one, so no need to
   * specify the target, for a client, the target is the server and vice versa
   **/
  void send(Message const &msg) {
    // Post asynchronously
    asio::post(pAsioContext, [this, msg]() {
      // The asio context may already be busy writing messages,
      // we will know if it is in the middle of such a task if the
      // out going message deque is not empty.
      bool currently_writing_messages = !pQmessagesOut.empty();
      // load our new message in
      pQmessagesOut.push_back(msg);
      if (!currently_writing_messages) {
        // wait until the deque of outgoing messages is empty to
        // start writing more
        this->writeStream_();
      }
    });
  }

 private:
  /**
   * @brief Async: Wraps asio::async_write
   *
   * @details Prime the context, ready to write a message body,
   * */
  void writeStream_() {
    //  Socket is not necessarily open here. Potentially causes 'Bad
    // descriptor' error.
    if (!pSocket.is_open()) {
      // TODO: Replace this with a light weight sys logger
      std::cerr << "Socket is closed, failed to write" << '\n';
      return;
    }
    asio::async_write(
        pSocket,
        asio::buffer(pQmessagesOut.front().mBody.data(),
                     pQmessagesOut.front().mBody.size()),
        [this](std::error_code error_code, u64 /*bytes_transferred*/) {
          if (!error_code || (error_code == asio::error::eof)) {
            // sending was successful, remove it from the deque
            pQmessagesOut.pop_front();

            // if the deque still has messages in it, call
            // write_header(), to start reading the next message
            if (!pQmessagesOut.empty()) {
              this->writeStream_();
            }
          } else {
            // sending the body failed, this could be an error, or perhaps
            // the other end of the connection disconnected. This is common
            // in tcp servers, the client will disconnect and the server does
            // not know about it until it tries to send something to the client.
            // TODO: Replace this with a light weight sys logger
            std::cerr << "\nError: " << error_code.message() << ":"
                      << error_code.value() << '\n';
            pSocket.close();
          }
        });
  }

  /**
   * @brief Async: Prime the context ready to read a message body
   * */
  void readStream_() {
    asio::async_read(
        pSocket, pReadBuffer, asio::transfer_at_least(1),
        [this](std::error_code error_code, u64 /*bytes_transferred*/) {
          if (!error_code) {
            // TODO: refactor for Packet and Message
            std::istream input(&pReadBuffer);
            std::string line{};
            std::string body{};
            while (std::getline(input, line)) {
              body.append(line);
            }
            pTempMsg << body;
            this->addToIncomingMessageQueue_();
          } else if (error_code == asio::error::eof) {
            // End of File
            std::cout << "Connection closed cleanly by peer" << '\n';
            this->disconnect();
          } else {
            // Error: reading the body failed
            // TODO: Replace this with a light weight sys logger
            std::cerr << "[" << pMetaData->mIpaddress << "]: "
                      << "Error: " << error_code.message() << ":"
                      << error_code.value() << 'n';
            pSocket.close();
          }
        });
  }

  /**
   * @brief If the owner of this connection is a server, we need to create an
   *  owned message,
   * */
  void addToIncomingMessageQueue_() {
    if (pOwnerType == Owner::server) {
      // NOTE: OwnedMessage requires a shared pointer, and a Message object,
      // since the Connection class inherits from shared_from_this, we can
      // pass in a shared pointer to this entire object, safer and more robust
      // than simply passing in *this.
      pQmessagesIn.push_back({this->shared_from_this(), pTempMsg});
    } else {
      // NOTE: if the connection belongs to a client, it makes no sense to take
      // the connection with a shared_ptr, as clients only have one connection
      pQmessagesIn.push_back({pTempMsg});
    }
    // NOTE: since this method is always called once we are finished reading
    // a message, we can use this opportunity to register another async task
    // for the asio context to perform
    readStream_();
  }

 private:
  /**
   * @brief Metadata for this connection
   * */
  ConnectionMetaDataPtr pMetaData{};

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
  containers::ThreadSafeDeque<OwnedMessage<T>> &pQmessagesIn{};

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
