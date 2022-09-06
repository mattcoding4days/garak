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

#ifndef LIBGARAK_NET_SERVER_HPP
#define LIBGARAK_NET_SERVER_HPP

#include <garak/net/connection.hpp>
#include <garak/net/error.hpp>

namespace garak::net {
/**
 * @brief Server. This interface is used to
 * create a server, and to accept connections from clients.
 * */
class Server {
 public:
  /**
   * @brief Construct a new Server object listening on the given port.
   *
   * @param port
   */
  explicit Server(u16 port);

  /**
   * @brief Destroy the Server object
   *
   */
  virtual ~Server();

  /**
   * @brief Because asios socket and context objects have deleted
   * copy constructors and move constructors, we must do the same
   * */
  Server(Server const &) = delete;
  Server &operator=(Server const &) = delete;
  Server(Server &&) = delete;
  Server &operator=(Server &&) = delete;

 public:
  /**
   * @brief Launch an asynchronous accept operation.
   *
   * @details This method is called by the server to start accepting
   * connections from clients. This is called recursively hence will continue
   * to accept new connections until the context is stopped.
   * */
  [[nodiscard]] bool startAcceptingConnections() noexcept;

  /**
   * @brief Run the server's event loop.
   *
   * @details This method is called by the server to start the event loop in
   * its own thread. The event loop will continue to run until the context is
   * stopped.
   **/
  void run() noexcept;

  /**
   * @brief if the asio context has to already been triggered to stop,
   * it will will be requested
   */
  void stop();

  /**
   * @brief async - instruct asio to wait for a connection
   *  -
   * https://think-async.com/Asio/boost_asio_1_20_0/doc/html/boost_asio/reference/basic_socket_acceptor/async_accept.html
   *
   * FIXME: the acceptor.async_accept class should be wrapped by a class of
   * our own, as there are many caveats to the object, and more fine grained
   * control/access will likely be needed.
   * -
   * boost asio book - Chapter 4 page 151
   * */
  void waitForConnection();

  /**
   * @brief Notify a single client connection with a response
   * @param The client connection to which the message object will be sent
   * @param msg object to send
   * */
  void notifyClient(std::shared_ptr<Connection> &client, const Message &msg);

  /**
   * @brief send a message to all clients
   * @param msg: The message to send
   * @param ignoreClient shared_ptr to a client to be ignored, defaults to
   * nullptr
   * */
  void notifyAllClients(const Message &msg,
                        std::shared_ptr<Connection> ignoreClient = nullptr);

  /**
   * @brief As the server gets busier and busier, it may be the case that
   * function would never return, as it would just constantly processing
   * messages added to the incoming messages deque, so the server side
   * application logic would likely never get updated. So we provide the user
   * a way to limit the amount of messages that it processes to force a
   * return, Basically, it is a method which performs user constrained batch
   * processing
   *
   * @param max_messages: the max number of messages to process in a single
   * batch. the default value is -1, which for an unsigned 64 bit int, that
   * is the largest possible value. User should define this
   * */

  // FIXME: Recommend using another value for `n_max_messages`, as it
  //  would be a better indicator semantically. The default value of -1 is
  //  not a good indicator of what the user is trying to do.
  void update(u64 max_messages = -1);

 protected:
  /**
   * @brief do some checking on a client, can ban the ip etc..
   * @param shared_ptr to the connection to inspect
   * */
  [[nodiscard]] virtual bool onClientConnect(
      std::shared_ptr<Connection> client) = 0;

  /**
   * @brief perform some clean up, and or book keeping when a client
   *  disconnects.
   * @param shared_ptr to the Connection client in which to clean up
   * */
  virtual void onClientDisconnect(std::shared_ptr<Connection> client) = 0;

  /**
   * @brief Called when a message arrives,allow the server to deal with a
   * reply message from a specific client
   * @param shared_ptr to the Connection client
   * @param Message object
   *
   * was "on_message()
   * */
  virtual void onNotify(std::shared_ptr<Connection> client, Message &msg) = 0;

 protected:
  /**
   * @brief The port in which the server is running on
   * */
  std::uint16_t mPort{};

  /**
   * @brief A client requires a thread safe queue for messages coming in
   * */
  containers::ThreadSafeDeque<OwnedMessage> mQueMessagesIn{};

  /**
   * @brief Container of all validated connections, it is a non-threadsafe
   * deque, so it should not be accessed by other threads.
   * */
  std::deque<std::shared_ptr<Connection>> mClientConnections{};

  /**
   * @brief The order of instantiation is important, first the io_context,
   * then the thread, as asio contexts need a thread
   * --
   * https://think-async.com/Asio/boost_asio_1_20_0/doc/html/boost_asio/reference/io_context.html
   */
  asio::io_context mAsioContext;

  /**
   * @brief the thread in which the io_context will run in
   */
  std::thread mThreadContext;

  /**
   * @brief Get the socket of the clients that are connected to this server
   *  --
   * https://think-async.com/Asio/boost_asio_1_20_0/doc/html/boost_asio/reference/ip__tcp/acceptor.html
   */
  asio::ip::tcp::acceptor mAsioAcceptor;
};

}  // namespace garak::net
#endif  // LIBGARAK_NET_SERVER_HPP
