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

#ifndef LIBGARAK_NET_MESSAGE_HPP
#define LIBGARAK_NET_MESSAGE_HPP

#include <garak/utils/module.hpp>

namespace garak::net {

/**
 * @brief Wrapper around an std::string
 *
 * @details Should the user need a server/client that is using jsonrpc
 * as its protocol, a universal string based messaging system would be
 * preferable instead of a low level byte steam.
 * */
class [[maybe_unused]] Message {
 public:
  std::string mBody{};

 public:
  Message() = default;
  virtual ~Message() = default;
  Message(Message const &) = default;
  Message &operator=(Message const &) = default;
  Message(Message &&) noexcept = default;
  Message &operator=(Message &&) noexcept = default;

 public:
  [[nodiscard]] std::size_t size() const noexcept { return mBody.size(); }

  /**
   * @brief Clear the message buffer
   */
  void clear() noexcept { mBody.clear(); }

 public:
  /**
   * @brief Override for console out compatibility/debug
   */
  friend std::ostream &operator<<(std::ostream &out_stream,
                                  Message const &msg) {
    out_stream << "Message: " << msg.mBody << "\nSize: " << msg.size();
    return out_stream;
  }

  /**
   * @brief Push data into the buffer, specialization overload for the
   * string type
   */
  friend Message &operator<<(Message &msg, std::string &data) {
    // sanitize
    u64 pos = data.find("\r\n\r\n");
    if (pos != std::string::npos) {
      data = data.substr(pos + 4);
    }
    msg.mBody = data.append("\n");
    return msg;
  }

  /**
   * @brief Remove data from the buffer, specialization overload for the
   * string type
   */
  friend Message &operator>>(Message &msg, std::string &data) {
    data = msg.mBody;
    std::size_t cache = (msg.size() - data.size());
    msg.mBody.resize(cache);
    return msg;
  }
};

template <class T>
class Connection;

/**
 * @brief An "owned" message is identical to a regular message, but it is
 * associated with a connection. On a server, the owner would be the client that
 * sent the message, on a client the owner would be the server.
 * */
template <class T>
class [[maybe_unused]] OwnedMessage {
 public:
  using OwnedConnection = std::shared_ptr<Connection<T>>;

 public:
  OwnedMessage() = default;
  [[maybe_unused]] OwnedMessage(OwnedConnection &&client, Message &msg)
      : mRemote(client), mOwnedMsg(msg) {}

  [[maybe_unused]] explicit OwnedMessage(Message &msg)
      : mRemote(nullptr), mOwnedMsg(msg) {}

  virtual ~OwnedMessage() = default;
  OwnedMessage(OwnedMessage const &) = default;
  OwnedMessage &operator=(OwnedMessage const &) = default;
  OwnedMessage(OwnedMessage &&) noexcept = default;
  OwnedMessage &operator=(OwnedMessage &&) noexcept = default;

 public:
  OwnedConnection mRemote{};
  Message mOwnedMsg{};
};
}  // namespace garak::net
#endif  // LIBGARAK_NET_MESSAGE_HPP
