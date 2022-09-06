//
// Created by matt on 9/5/22.
//

#include <garak/net/connection.hpp>

namespace garak::net {

/************************* ConnectionMetaData ********************************/
ConnectionMetaData::ConnectionMetaData(std::string ipaddress, u16 port)
    : pIpaddress(std::move(ipaddress)), pPort(port) {}

u16 ConnectionMetaData::getPort() const noexcept { return pPort; }

std::string ConnectionMetaData::getIpAddress() const noexcept {
  return pIpaddress;
}

std::string ConnectionMetaData::basicInfo() const noexcept {
  return std::string{pIpaddress}.append(":").append(std::to_string(pPort));
}

std::optional<std::string> ConnectionMetaData::getAdditionalInfoByKey(
    std::string const &key) const noexcept {
  std::optional<std::string> value{std::nullopt};
  if (pAdditionalInfo.contains(key)) {
    // This is a paranoid try/catch as contains is a new feature.
    value.emplace(pAdditionalInfo.at(key));
  }
  return value;
}

void ConnectionMetaData::updateAdditionalInfo(
    ConnectionMetaData::AdditionalInfo &info) noexcept {
  pAdditionalInfo.insert({std::move(info.mKey), std::move(info.mValue)});
}

void ConnectionMetaData::updateAdditionalInfo(
    ConnectionMetaData::AdditionalInfo &&info) noexcept {
  pAdditionalInfo.insert({std::move(info.mKey), std::move(info.mValue)});
}

/**************************** Connection **************************************/
Connection::Connection(ConnectionMetaData metaData, Owner parent,
                       asio::io_context &asioContext,
                       asio::ip::tcp::socket socket,
                       containers::ThreadSafeDeque<OwnedMessage> &incomingQueue)
    : pMetaData(std::move(metaData)),
      pOwnerType(parent),
      pAsioContext(asioContext),
      pSocket(std::move(socket)),
      pQmessagesIn(incomingQueue) {}

std::string Connection::getIp() const noexcept {
  return pMetaData.getIpAddress();
}

u16 Connection::getPort() const noexcept { return pMetaData.getPort(); }

ConnectionMetaData &Connection::getConnectionMetaData() noexcept {
  return pMetaData;
}

void Connection::connectToClient() noexcept {
  if (pOwnerType == Owner::server) {
    if (pSocket.is_open()) {
      this->readStream_();
    }
  }
}

void Connection::connectToServer(
    asio::ip::tcp::resolver::results_type const &endpoint) {
  if (pOwnerType == Owner::client) {
    // request asio attempts to connect to an endpoint
    asio::async_connect(pSocket, endpoint,
                        [this](std::error_code error_code,
                               asio::ip::tcp::endpoint const & /*endpoint*/) {
                          if (!error_code) {
                            this->readStream_();
                          } else {
                            throw ConnectionError(error_code.message(),
                                                  error::RUNTIME_INFO);
                          }
                        });
  }
}

void Connection::disconnect() {
  if (this->isConnected()) {
    asio::post(pAsioContext, [this]() { pSocket.close(); });
  }
}

bool Connection::isConnected() const noexcept { return pSocket.is_open(); }

void Connection::send(Message const &msg) {
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

void Connection::writeStream_() {
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
          std::cerr << "[" << pMetaData.basicInfo() << "]: "
                    << "Error: " << error_code.message() << ":"
                    << error_code.value() << 'n';
          pSocket.close();
        }
      });
}

void Connection::readStream_() {
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
          std::cerr << "[" << pMetaData.basicInfo() << "]: "
                    << "Error: " << error_code.message() << ":"
                    << error_code.value() << 'n';
          pSocket.close();
        }
      });
}

void Connection::addToIncomingMessageQueue_() {
  if (pOwnerType == Owner::server) {
    // NOTE: OwnedMessage requires a shared pointer, and a Message object,
    // since the Connection class inherits from shared_from_this, we can
    // pass in a shared pointer to this entire object, safer and more robust
    // than simply passing in *this.
    pQmessagesIn.push_back({this->shared_from_this(), pTempMsg});
  } else {
    // NOTE: if the connection belongs to a client, it makes no sense to take
    // the connection with a shared_ptr, as clients only have one connection
    pQmessagesIn.push_back(OwnedMessage(pTempMsg));
  }
  // NOTE: since this method is always called once we are finished reading
  // a message, we can use this opportunity to register another async task
  // for the asio context to perform
  readStream_();
}
}  // namespace garak::net