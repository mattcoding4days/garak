//
// Created by matt on 9/4/22.
//

#ifndef LIBGARAK_NET_INTERFACES_HPP
#define LIBGARAK_NET_INTERFACES_HPP

#include <garak/utils/module.hpp>

namespace garak::net {
/**
 * @brief Interface for Meta data to represent a connection in a more
 * descriptive way.
 *
 * @details A user may want to attach more information to a connection that just
 * an id and an ip address. So this interface is meant to be inherited inorder,
 * to add more functionality to it if necessary.
 * */
class IConnectionMetaData {
 public:
  IConnectionMetaData() = default;
  [[maybe_unused]] IConnectionMetaData(std::string ipaddress, u8 port)
      : mIpaddress(std::move(ipaddress)), mPort(port) {}

  virtual ~IConnectionMetaData() = default;
  IConnectionMetaData(IConnectionMetaData const &) = default;
  IConnectionMetaData &operator=(IConnectionMetaData const &) = default;
  IConnectionMetaData(IConnectionMetaData &&) noexcept = default;
  IConnectionMetaData &operator=(IConnectionMetaData &&) noexcept = default;

 public:
  std::string mIpaddress{};
  u8 mPort{};
};

using ConnectionMetaDataPtr = std::shared_ptr<IConnectionMetaData>;
}  // namespace garak::net

#endif  // LIBGARAK_NET_INTERFACES_HPP
