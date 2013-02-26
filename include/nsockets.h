#pragma once

namespace nsockets {

  using std::wstring;

  enum Protocol {
    Protocol_Any = 0,
    Protocol_IPv4,
    Protocol_IPv6,
    Protocol_Unknown
  };

  namespace util {
    inline Protocol familyToProtocol( int family );
    inline int protocolToFamily( Protocol protocol );
  }

  struct ConnectionInfo {
  public:
    WCHAR hostAddress[NI_MAXHOST];
    WCHAR hostService[NI_MAXSERV];
    SOCKADDR_STORAGE socketAddress;
    ADDRINFOW addressInfo;
    void getFrom( SOCKET socket, PADDRINFOW address, bool remote );
  };

  class Socket {
  protected:
    volatile SOCKET mSocket;
    ConnectionInfo mConnectionInfo;
  public:
    Socket();
    const Protocol getProtocol();
    const ConnectionInfo& getConnectionInfo();
  };

  class TCPSocket: public Socket {
  protected:
    volatile WSAEVENT mNetworkEvent;
  public:
    TCPSocket();
    virtual ~TCPSocket();
    virtual void bind( const wstring& host, const wstring& service, Protocol protocol = Protocol_Any );
    virtual void listen();
    virtual void connect( const wstring& host, const wstring& service, Protocol protocol = Protocol_Any );
    virtual void closeRequest();
    virtual void close();
  };

}