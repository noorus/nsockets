#pragma once

namespace nsockets {

  enum Protocol {
    Protocol_Any = 0,
    Protocol_IPv4,
    Protocol_IPv6
  };

  struct ConnectionInfo {
  public:
    WCHAR hostAddress[NI_MAXHOST];
    WCHAR hostService[NI_MAXSERV];
    SOCKADDR_STORAGE socketAddress;
    ADDRINFOW addressInfo;
  };

  class Socket {
  protected:
    volatile SOCKET wsSocket;
    ConnectionInfo mConnectionInfo;
  public:
    Socket();
  };

}