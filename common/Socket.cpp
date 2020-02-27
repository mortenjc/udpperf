/** Copyright (C) 2016 European Spallation Source */

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <common/Socket.h>
#include <netdb.h>

/// \brief Use MSG_SIGNAL on Linuxes
#ifdef MSG_NOSIGNAL
#define SEND_FLAGS MSG_NOSIGNAL
#else
#define SEND_FLAGS 0
#endif


bool Socket::isValidIp(std::string ipAddress) {
  struct sockaddr_in SockAddr;
  return inet_pton(AF_INET, ipAddress.c_str(), &(SockAddr.sin_addr)) != 0;
}

std::string Socket::getHostByName(std::string &name) {
  hostent * HostEntry = gethostbyname(name.c_str());
  if (HostEntry == nullptr) {
    throw std::runtime_error("Unable to resolve hostname");
  } else { // Just return the first entry
    auto IpAddress = inet_ntoa(*reinterpret_cast<in_addr*>(HostEntry->h_addr_list[0]));
    //LOG(IPC, Sev::Info, "Hostname resolved to {}", IpAddress);
    return IpAddress;
  }
}

Socket::Socket(Socket::type stype) {
  auto type = (stype == Socket::type::UDP) ? SOCK_DGRAM : SOCK_STREAM;
  auto proto = (stype == Socket::type::UDP) ? IPPROTO_UDP : IPPROTO_TCP;

  if ((SocketFileDescriptor = socket(AF_INET, type, proto)) == -1) {
    //LOG(IPC, Sev::Error, "socket() failed");
    throw std::runtime_error("system error - socket() failed");
  }
}

int Socket::setBufferSizes(int sndbuf, int rcvbuf) {
  if (sndbuf) {
    setSockOpt(SO_SNDBUF, &sndbuf, sizeof(sndbuf));
  }
  if (rcvbuf) {
    setSockOpt(SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));
  }
  return 0; // setsockopt for SO_SND/RCVBUFon Linux cannot fail.
}

void Socket::getBufferSizes(int & sendBuffer, int & receiveBuffer) {
  sendBuffer = getSockOpt(SO_SNDBUF);
  receiveBuffer = getSockOpt(SO_RCVBUF);
}

void Socket::printBufferSizes(void) {
  //LOG(IPC, Sev::Info, "Socket receive buffer size: {}", getSockOpt(SO_RCVBUF));
  //LOG(IPC, Sev::Info, "Socket send buffer size: {}", getSockOpt(SO_SNDBUF));
}

void Socket::checkRxBufferSizes(std::int32_t MinRxBufferSize) {
  int __attribute__((unused)) TxBufferSize;
  int RxBufferSize;
  getBufferSizes(TxBufferSize, RxBufferSize);
  if (RxBufferSize < MinRxBufferSize) {
    //LOG(IPC, Sev::Warning,
    //   fmt::format("receive buffer size error. expected >= {}, got {}",
    //               MinRxBufferSize, RxBufferSize));
  }
}

int Socket::setRecvTimeout(int seconds, int usecs) {
  struct timeval timeout;
  timeout.tv_sec = seconds;
  timeout.tv_usec = usecs;
  return setSockOpt(SO_RCVTIMEO, &timeout, sizeof(timeout));
}

int Socket::setNOSIGPIPE() {
#ifdef SYSTEM_NAME_DARWIN
    //LOG(IPC, Sev::Info, "setsockopt() - MacOS specific");
    int on = 1;
    int ret = setSockOpt(SO_NOSIGPIPE, &on, sizeof(on));
    if (ret != 0) {
        //LOG(IPC, Sev::Warning,
        //    fmt::format("Cannot set SO_NOSIGPIPE for socket: {}", strerror(ret)).c_str() );
    }
    assert(ret ==0);
    return ret;
#else
    return 0;
#endif
}

void Socket::setLocalSocket(const std::string ipaddr, int port) {
  // zero out the structures
  struct sockaddr_in localSockAddr;
  std::memset((char *)&localSockAddr, 0, sizeof(localSockAddr));
  localSockAddr.sin_family = AF_INET;
  localSockAddr.sin_port = htons(port);

  int ret = inet_aton(ipaddr.c_str(), &localSockAddr.sin_addr);
  if (ret == 0) {
    throw std::runtime_error("setLocalSocket() - invalid ip address");
  }

  // bind socket to port
  ret = bind(SocketFileDescriptor, (struct sockaddr *)&localSockAddr, sizeof(localSockAddr));
  if (ret != 0) {
    throw std::runtime_error("setLocalSocket(): bind failed, is port already in use?");
  }
}

void Socket::setRemoteSocket(const std::string ipaddr, int port) {
  RemoteIp = ipaddr;
  RemotePort = port;
  // zero out the structures
  std::memset((char *)&remoteSockAddr, 0, sizeof(remoteSockAddr));
  remoteSockAddr.sin_family = AF_INET;
  remoteSockAddr.sin_port = htons(port);

  int ret = inet_aton(ipaddr.c_str(), &remoteSockAddr.sin_addr);
  if (ret == 0) {
    throw std::runtime_error("RemoteSocket(): invalid ip address");
  }
}

int Socket::connectToRemote() {
  // zero out the structures
  struct sockaddr_in remoteSockAddr;
  std::memset((char *)&remoteSockAddr, 0, sizeof(remoteSockAddr));
  remoteSockAddr.sin_family = AF_INET;
  remoteSockAddr.sin_port = htons(RemotePort);
  int ret = inet_aton(RemoteIp.c_str(), &remoteSockAddr.sin_addr);
  if (ret == 0) {
    //LOG(IPC, Sev::Error, "invalid ip address {}", RemoteIp);
    throw std::runtime_error("connectToRemote() - invalid ip");
  }

  ret = connect(SocketFileDescriptor, (struct sockaddr *)&remoteSockAddr, sizeof(remoteSockAddr));
  if (ret < 0) {
    //LOG(IPC, Sev::Error, "connect() to {}:{} failed", RemoteIp, RemotePort);
    SocketFileDescriptor = -1;
  }
  return ret;
}

int Socket::send(void const *buffer, int len) {
  //XTRACE(IPC, DEB, "Socket::send(), length %d bytes", len);
  int ret =
      sendto(SocketFileDescriptor, buffer, len, SEND_FLAGS, (struct sockaddr *)&remoteSockAddr, sizeof(remoteSockAddr));
  if (ret < 0) {
    SocketFileDescriptor = -1;
    //XTRACE(IPC, DEB, "sendto() failed with code %d", ret);
  }

  return ret;
}

/** */
ssize_t Socket::receive(void *buffer, int buflen) {
  socklen_t slen = 0;
  // try to receive some data, this is a blocking call
  return recvfrom(SocketFileDescriptor, buffer, buflen, 0, (struct sockaddr *)&remoteSockAddr, &slen);
}

//
// Private methods
//

int Socket::getSockOpt(int option) {
  //XTRACE(IPC, DEB, "getSockOpt(%d), fd %d", option, SocketFileDescriptor);
  int optval, ret;
  socklen_t optlen;
  optlen = sizeof(optval);
  if ((ret = getsockopt(SocketFileDescriptor, SOL_SOCKET, option, (void *)&optval, &optlen)) <
      0) {
    //XTRACE(IPC, WAR, "getSockOpt(%d) failed, fd %d, ret %d", option, SocketFileDescriptor, ret);
    return ret;
  }
  return optval;
}

int Socket::setSockOpt(int option, void *value, int size) {
  int ret;
  if ((ret = setsockopt(SocketFileDescriptor, SOL_SOCKET, option, value, size)) < 0) {
    std::cout << "setsockopt() failed" << std::endl;
  }
  return ret;
}

bool Socket::isValidSocket() {
  return (SocketFileDescriptor >= 0);
}

///
///
///
TCPTransmitter::TCPTransmitter(const std::string ipaddr, int port) : Socket(Socket::type::TCP) {
  setRemoteSocket(ipaddr, port);
  setNOSIGPIPE();
  connectToRemote();
}

int TCPTransmitter::senddata(char const *buffer, int len) {
  if (!isValidSocket()) {
    //XTRACE(IPC, WAR, "No file descriptor for TCP transmitter");
    return -1;
  }

  if (len <= 0) {
    //LOG(IPC, Sev::Warning, "TCPClient::senddata() no data specified");
    return 0;
  }

  return send(buffer, len);
}
