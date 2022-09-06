//
// Created by matt on 9/5/22.
//
#include <garak/net/message.hpp>

namespace garak::net {

u64 Message::size() const noexcept { return mBody.size(); }

void Message::clear() noexcept { mBody.clear(); }

std::ostream &operator<<(std::ostream &out_stream, Message const &msg) {
  out_stream << "Message: " << msg.mBody << "\nSize: " << msg.size();
  return out_stream;
}

Message &operator<<(Message &msg, std::string &data) {
  // sanitize
  u64 pos = data.find("\r\n\r\n");
  if (pos != std::string::npos) {
    data = data.substr(pos + 4);
  }
  msg.mBody = data.append("\n");
  return msg;
}

Message &operator>>(Message &msg, std::string &data) {
  data = msg.mBody;
  std::size_t cache = (msg.size() - data.size());
  msg.mBody.resize(cache);
  return msg;
}

OwnedMessage::OwnedMessage(OwnedConnectionPtr &&client, Message &msg)
    : mRemote(client), mOwnedMsg(msg) {}

OwnedMessage::OwnedMessage(Message &msg) : mRemote(nullptr), mOwnedMsg(msg) {}
}  // namespace garak::net
