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

#ifndef LIBGARAK_NET_PACKET_HPP
#define LIBGARAK_NET_PACKET_HPP

#include <cstring>
#include <garak/utils/module.hpp>

namespace garak::net {

/**
 * @brief Represent the header of a byte packet
 * */
template <class T>
class PacketHeader {
 public:
  T mId{};
  u32 mSize{};
};

/**
 * @brief Represent a Packet of Bytes
 *
 * @details this supports more low-level messaging
 * protocols that might be used. For now libgarak will only support strings,
 * via the Message class
 * */
template <class T>
class [[maybe_unused]] Packet {
 public:
  using PacketStream = std::vector<u8>;

 public:
  PacketHeader<T> mHeader{};
  PacketStream mBody{};

  Packet() = default;
  virtual ~Packet() = default;
  Packet(Packet const &) = default;
  Packet &operator=(Packet const &) = default;
  Packet(Packet &&) noexcept = default;
  Packet &operator=(Packet &&) noexcept = default;

 public:
  /**
   * @brief Returns the size of the message body
   * */
  [[nodiscard]] u64 size() const noexcept { return mBody.size(); }

 public:
  /**
   * @brief Override for console out compatibility/debugging
   */
  friend std::ostream &operator<<(std::ostream &out_stream,
                                  const Packet &packet) {
    out_stream << "Id:" << int(packet.mHeader.mId)
               << " Size:" << packet.mHeader.mSize;
    return out_stream;
  }

  /**
   * @brief Pushes any POD-like data into the message buffer
   */
  template <class DataType>
  friend Packet &operator<<(Packet &packet, DataType const &data) {
    // Check that the type of the data being pushed is trivially copyable
    static_assert(std::is_standard_layout<DataType>::value,
                  "Data is too complex to be inserted into a vector<u8>");

    // Cache current size of vector, as this will be the point we insert the
    // data
    u64 cached_size = packet.mBody.size();

    // Resize the vector by the size of the data being pushed
    packet.mBody.resize(packet.mBody.size() + sizeof(DataType));

    // Physically copy the data into the newly allocated vector space,
    // TODO: Replace memcpy with iterators
    std::memcpy(packet.mBody.data() + cached_size, &data, sizeof(DataType));

    // Recalculate the message size
    packet.mHeader.mSize = packet.size();

    // Return the target packet, so it can be "chained"
    return packet;
  }

  /**
   * @brief Pulls any POD-like data form the message buffer
   * */
  template <typename DataType>
  friend Packet<T> &operator>>(Packet<T> &packet, DataType &data) {
    // Check that the type of the data being pushed is trivially copyable
    static_assert(std::is_standard_layout<DataType>::value,
                  "Data is too complex to be extracted from a vector<u8>");

    // Cache the location towards the end of the vector where the pulled data
    // starts
    u64 cached_size = packet.body.size() - sizeof(DataType);

    // Physically copy the data from the vector into the user variable
    // TODO: Replace memcpy with iterators
    std::memcpy(&data, packet.mBody.data() + cached_size, sizeof(DataType));

    // Shrink the vector to remove read bytes, and reset end position
    packet.mBody.resize(cached_size);

    // Recalculate the message size
    packet.mHeader.mSize = packet.size();

    // Return the target packet, so it can be "chained"
    return packet;
  }
};

template <class T>
class Connection;

/**
 * @brief An "owned" packet is identical to a regular packet, but it is
 * associated with a connection. On a server, the owner would be the client that
 * sent the message, on a client the owner would be the server.
 * */
template <class T>
class [[maybe_unused]] OwnedPacket {
 public:
  using OwnedConnection = std::shared_ptr<Connection<T>>;

 public:
  OwnedConnection mRemote{};
  Packet<T> mPacket{};

 public:
  OwnedPacket() = default;
  [[maybe_unused]] OwnedPacket(OwnedConnection &&client, Packet<T> &packet)
      : mRemote(client), mPacket(packet) {}

  [[maybe_unused]] explicit OwnedPacket(Packet<T> &packet)
      : mRemote(nullptr), mPacket(packet) {}

  virtual ~OwnedPacket() = default;
  OwnedPacket(OwnedPacket const &) = default;
  OwnedPacket &operator=(OwnedPacket const &) = default;
  OwnedPacket(OwnedPacket &&) noexcept = default;
  OwnedPacket &operator=(OwnedPacket &&) noexcept = default;

 public:
  /**
   * @brief Override for console out compatibility/debugging
   */
  friend std::ostream &operator<<(std::ostream &out_stream,
                                  const OwnedPacket<T> &owned_packet) {
    out_stream << owned_packet.mPacket;
    return out_stream;
  }
};
}  // namespace garak::net

#endif  // LIBGARAK_NET_PACKET_HPP
