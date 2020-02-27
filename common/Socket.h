/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the declaration of the Socket abstration for BSD sockets
/// it is used in detector pipeline plugins for receive and in udptx for transmit
///
//===----------------------------------------------------------------------===//

#pragma once

#include <arpa/inet.h>
#include <cassert>
#include <cinttypes>
#include <sys/socket.h>
#include <netinet/ip.h>

/// BSD Socket abstractions for TCP and UDP transmitters and receivers
class Socket {
public:
  enum class type { UDP, TCP };
  static const int buflen_max = 9000;

  class Endpoint {
  public:
    const std::string ipaddr;
    uint16_t port;
    Endpoint(const std::string ip_address, uint16_t port_number)
        : ipaddr(ip_address), port(port_number) {}
  };

  /// \brief Is this a dotted quad ip address?
  /// Valid addresses must be of the form 'a.b.d.c' where
  /// a-d can range from 0 to 255
  static bool isValidIp(std::string ipAddress);

  /// \brief Return dotted quad by resolving hostname
  /// Essentially a wrapper for gethostbyname() returning
  /// the first entry in the ip address table
  static std::string getHostByName(std::string &name);

  /// Create a socker abstraction of type UDP or TCP
  Socket(Socket::type type);

  /// Attempt to specify the socket receive and transmit buffer sizes (for performance)
  int setBufferSizes(int sndbuf, int rcvbuf);

  /// Get tx and rx buffer sizes
  void getBufferSizes(int & sendBuffer, int & receiveBuffer);

  /// Check that buffer sizes meet expectations
  void checkRxBufferSizes(std::int32_t MinRxBufferSize);

  /// Print the current values for receive and trasmit buffer sizes
  void printBufferSizes(void);

  /// Set a timeout for recv() function rather than wait for ever
  int setRecvTimeout(int seconds, int usecs);

  /// Set socket option (Mac only) for not sending SIGPIPE on transmitting on invalid socket
  int setNOSIGPIPE();

  /// Specify ip address of interface to receive data on and port number to listen on
  void setLocalSocket(const std::string ipaddr, int port);

  /// Specify ip address and port number of remote end
  void setRemoteSocket(const std::string ipaddr, int port);

  /// Connect (TCP only) to remote endpoint
  int connectToRemote();

  /// Receive data on socket into buffer with specified length
  ssize_t receive(void *receiveBuffer, int bufferSize);

  /// Send data in buffer with specified length
  int send(void const *dataBuffer, int dataLength);

  /// \brief To check if data can be transmitted or received
  bool isValidSocket();

private:
  int SocketFileDescriptor{-1};
  std::string RemoteIp;
  int RemotePort;
  struct sockaddr_in remoteSockAddr;

  /// wrapper for getsockopt() system call
  int getSockOpt(int option);

  /// wrapper for setsockopt() system call
  int setSockOpt(int option, void *value, int size);
};

/// UDP receiver only needs to specify local socket
class UDPReceiver : public Socket {
public:
  UDPReceiver(Endpoint local) : Socket(Socket::type::UDP) {
    this->setLocalSocket(local.ipaddr, local.port);
  };
};

/// UDP transmitter needs to specify both local and remote socket
class UDPTransmitter : public Socket {
public:
  UDPTransmitter(Endpoint local, Endpoint remote) : Socket(Socket::type::UDP) {
    this->setLocalSocket(local.ipaddr, local.port);
    this->setRemoteSocket(remote.ipaddr, remote.port);
  };
};

class TCPTransmitter : public Socket {
public:
  ///
  TCPTransmitter(const std::string ip, int port);

  ///
  int senddata(char const *buffer, int len);
};
