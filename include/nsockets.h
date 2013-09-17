#pragma once

#define NTDDI_VERSION NTDDI_WS08SP4
#define _WIN32_WINNT _WIN32_WINNT_WS08
#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdint.h>
#include <wchar.h>

#include <exception>
#include <string>
#include <vector>
#include <list>
#include <sstream>
#include <boost/variant.hpp>

namespace nsockets {

  using std::wstring;
  using std::string;
  using std::list;
  using std::vector;
  using std::wstringstream;
  using boost::variant;

  //! \class WinAPIError
  //! Container for a Windows API error description.
  struct WinAPIError {
  public:
    uint32_t code;
    wstring description;
  };

  //! \class Exception
  //! Main exception class for nsocket errors. Descendant of std::exception.
  class Exception: public std::exception {
  public:
    enum Type {
      Generic = 0,
      Winsock
    };
  private:
    Exception();
  protected:
    Type mType;
    wstring mDescription;
    wstring mSource;
    mutable wstring mFullDescription;
    variant<WinAPIError> mAdditional;
    void handleAdditional();
  public:
    Exception( const wstring& description, Type type = Generic );
    Exception( const wstring& description, const wstring& source, Type type = Generic );
    virtual const wstring& getFullDescription() const;
    virtual const char* what() const throw();
  };

  enum Protocol: uint32_t {
    Protocol_Any = 0,
    Protocol_IPv4,
    Protocol_IPv6,
    Protocol_Unknown
  };

  //! \class ConnectionIfo
  //! Socket information container.
  struct ConnectionInfo {
  public:
    WCHAR hostAddress[NI_MAXHOST];
    WCHAR hostService[NI_MAXSERV];
    SOCKADDR_STORAGE socketAddress;
    ADDRINFOW addressInfo;
    void getFrom( SOCKET socket, PADDRINFOW address, bool remote );
  };

  class Socket;

  //! \class SocketListener
  //! Pure virtual nsocket listener base class.
  //! Listeners are used to handle socket events.
  class SocketListener {
  public:
    virtual void connectCallback( Socket* socket ) = 0;
    virtual void readCallback( Socket* socket ) = 0;
    virtual void closeCallback( Socket* socket ) = 0;
  };

  typedef list<SocketListener*> SocketListenerList;

  //! \class Socket
  //! Non-specific socket base class.
  class Socket {
  protected:
    volatile SOCKET mSocket; //!< Winsock socket handle
    ConnectionInfo mConnectionInfo; //!< Connection information struct
    SocketListenerList mListeners; //!< List of subscribed listeners
  public:
    Socket();
    SOCKET getRawSocket();
    const Protocol getProtocol();
    const ConnectionInfo& getConnectionInfo();
    virtual void addListener( SocketListener* listener );
    virtual void removeListener( SocketListener* listener );
  };

  //! \class TCPSocket
  //! TCP socket class. Descendant of Socket.
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
    State mState; //!< Socket state
    CloseReason mCloseReason;
    struct Errors {
      int closeEventError;
      int readEventError;
    } mErrors;
  public:
    TCPSocket();
    virtual ~TCPSocket();
    virtual const State& getState();
    virtual void bind( const wstring& host, const wstring& service,
      Protocol protocol = Protocol_Any );
    virtual void listen();
    virtual void connect( const wstring& host, const wstring& service,
      Protocol protocol = Protocol_Any );
    virtual uint32_t write( const void* buffer, const uint32_t length );
    virtual uint32_t read( void* buffer, uint32_t length );
    virtual void process();
    virtual void closeRequest();
    virtual void close();
  };

  extern WSADATA g_wsaData;

  void initialize(); //!< Initialize Winsock for nsockets usage.
  void shutdown(); //!< Shutdown Winsock after usage.

}