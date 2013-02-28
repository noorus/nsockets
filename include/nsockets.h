#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdint.h>

#include <exception>
#include <string>
#include <vector>
#include <list>

namespace nsockets {

  using std::wstring;
  using std::list;

  enum Protocol: uint32_t {
    Protocol_Any = 0,
    Protocol_IPv4,
    Protocol_IPv6,
    Protocol_Unknown
  };

  struct ConnectionInfo {
  public:
    WCHAR hostAddress[NI_MAXHOST];
    WCHAR hostService[NI_MAXSERV];
    SOCKADDR_STORAGE socketAddress;
    ADDRINFOW addressInfo;
    void getFrom( SOCKET socket, PADDRINFOW address, bool remote );
  };

  class Socket;

  class SocketListener {
  public:
    virtual void readCallback( Socket* socket ) = 0;
    virtual void closeCallback( Socket* socket ) = 0;
  };

  typedef list<SocketListener*> SocketListenerList;

  class Socket {
  protected:
    volatile SOCKET mSocket;
    ConnectionInfo mConnectionInfo;
    SocketListenerList mListeners;
  public:
    Socket();
    SOCKET getRawSocket();
    const Protocol getProtocol();
    const ConnectionInfo& getConnectionInfo();
    virtual void addListener( SocketListener* listener );
    virtual void removeListener( SocketListener* listener );
  };

  class TCPSocket: public Socket {
  public:
    enum State: uint32_t {
      State_Disconnected = 0,
      State_Connecting,
      State_Connected,
      State_Disconnecting,
      State_Listening
    };
    enum CloseReason {
      Close_Unexpected = 0,
      Close_Graceful,
      Close_Error
    };
  protected:
    volatile WSAEVENT mNetworkEvent;
    State mState;
    CloseReason mCloseReason;
    struct Errors {
      int closeEventError;
      int readEventError;
    } mErrors;
  public:
    TCPSocket();
    virtual ~TCPSocket();
    virtual const State& getState();
    virtual void bind( const wstring& host, const wstring& service, Protocol protocol = Protocol_Any );
    virtual void listen();
    virtual void connect( const wstring& host, const wstring& service, Protocol protocol = Protocol_Any );
    virtual uint32_t write( const void* buffer, const uint32_t length );
    virtual uint32_t read( void* buffer, uint32_t length );
    virtual void process();
    virtual void closeRequest();
    virtual void close();
  };

}