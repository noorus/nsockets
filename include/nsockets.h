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

  enum Protocol {
    Protocol_Any = 0,
    Protocol_IPv4,
    Protocol_IPv6,
    Protocol_Unknown
  };

  //! \class ConnectionIfo
  //! Socket information container.
  struct ConnectionInfo {
  protected:
    wstring mRemoteAddress;
    wstring mRemoteService;
    wstring mLocalAddress;
    wstring mLocalService;
    Protocol mProtocol;
    void reset();
  public:
    const wstring& getRemoteAddress() const throw();
    const wstring& getRemoteService() const throw();
    const wstring& getLocalAddress() const throw();
    const wstring& getLocalService() const throw();
    const Protocol getProtocol() const throw();
  public:
    ConnectionInfo();
    void update( SOCKET socket, bool remote );
  };

  class Socket;

  //! \class SocketListener
  //! Pure virtual socket listener base class.
  //! Listeners are used to handle socket events.
  class SocketListener {
  public:
    virtual bool acceptCallback( Socket* socket ) = 0;
    virtual bool connectCallback( Socket* socket ) = 0;
    virtual bool readCallback( Socket* socket ) = 0;
    virtual bool closeCallback( Socket* socket ) = 0;
  };

  typedef list<SocketListener*> SocketListenerList;

  //! \class Socket
  //! Non-specific socket base class.
  class Socket {
  protected:
    volatile SOCKET mSocket; //!< Winsock socket handle
    ConnectionInfo mConnectionInfo; //!< Connection information struct
    SocketListenerList mListeners; //!< List of subscribed listeners
    Socket();
  public:
    SOCKET getRawSocket() const throw();
    const Protocol getProtocol() const throw();
    const ConnectionInfo& getConnectionInfo();
    virtual void addListener( SocketListener* listener );
    virtual void removeListener( SocketListener* listener );
  };

  //! \class UDPSocket
  //! UDP socket class. Descendant of Socket.
  class UDPSocket: public Socket {
  public:
    UDPSocket();
    virtual ~UDPSocket();
    virtual void setReceiver( const wstring& host, const wstring& service,
      Protocol protocol = Protocol_Any );
  };

  //! \class TCPSocket
  //! TCP socket base class. Descendant of Socket.
  class TCPSocket: public Socket {
  public:
    enum State {
      State_Closed = 0,
      State_Connecting,
      State_Connected,
      State_Closing,
      State_Listening
    };
    enum CloseReason {
      Close_Unexpected = 0,
      Close_Graceful,
      Close_Error
    };
  protected:
    State mState; //!< Socket state
    CloseReason mCloseReason;
    struct Errors {
      int acceptEventError;
      int closeEventError;
      int readEventError;
    } mErrors;
    bool mOverlapped;
    TCPSocket( bool overlapped );
  public:
    virtual ~TCPSocket();
    virtual const State& getState();
    virtual void bind( const wstring& host, const wstring& service,
      Protocol protocol = Protocol_Any );
    virtual void listen() = 0;
    virtual void accept( TCPSocket* socket ) = 0;
    virtual void connect( const wstring& host, const wstring& service,
      Protocol protocol ) = 0;
    virtual uint32_t write( const void* buffer, const uint32_t length );
    virtual uint32_t read( void* buffer, uint32_t length );
    virtual void closeRequest();
    virtual void close();
  };

  //! \class EventTCPSocket
  //! Network event-based TCP socket class. Descendant of TCPSocket.
  class EventTCPSocket: public TCPSocket {
  protected:
    volatile WSAEVENT mNetworkEvent;
  public:
    EventTCPSocket( bool overlapped = false );
    virtual ~EventTCPSocket();
    virtual void listen();
    virtual void accept( TCPSocket* socket );
    virtual void connect( const wstring& host, const wstring& service,
      Protocol protocol = Protocol_Any );
    virtual void process();
  };

  extern WSADATA g_wsaData;

  void initialize(); //!< Initialize Winsock for nsockets usage.
  void shutdown(); //!< Shutdown Winsock after usage.

}