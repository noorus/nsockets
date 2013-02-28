#include "nsockets.h"
#include "nsocketsUtil.h"

namespace nsockets {

  TCPSocket::TCPSocket(): Socket(),
  mNetworkEvent( WSA_INVALID_EVENT ), mState( State_Disconnected )
  {
    mNetworkEvent = WSACreateEvent();
    if ( mNetworkEvent == WSA_INVALID_EVENT )
      throw new std::exception( "Couldn't create network event" );
    memset( &mErrors, 0, sizeof( Errors ) );
  }

  const TCPSocket::State& TCPSocket::getState()
  {
    return mState;
  }

  void TCPSocket::bind( const wstring& host, const wstring& service,
  Protocol protocol )
  {
    if ( mState != State_Disconnected )
      throw new std::exception( "Cannot bind, socket is not disconnected" );

    ADDRINFOW resolve;
    PADDRINFOW address = nullptr;
    PADDRINFOW resolved = nullptr;

    memset( &resolve, 0, sizeof( ADDRINFOW ) );

    resolve.ai_family   = util::protocolToFamily( protocol );
    resolve.ai_socktype = SOCK_STREAM;
    resolve.ai_protocol = IPPROTO_TCP;

    if ( GetAddrInfoW( host.c_str(), service.c_str(), &resolve, &resolved ) )
      throw new std::exception( "Couldn't resolve" );

    for ( address = resolved; address != nullptr; address = address->ai_next )
    {
      if ( address->ai_socktype != SOCK_STREAM || address->ai_protocol != IPPROTO_TCP )
        continue;
      mSocket = WSASocketW(
        address->ai_family, address->ai_socktype, address->ai_protocol,
        nullptr, 0, 0 );
      if ( mSocket == INVALID_SOCKET )
        continue;
      if ( ::bind( mSocket, address->ai_addr, (int)address->ai_addrlen ) != SOCKET_ERROR )
        break;
      closesocket( mSocket );
    }

    if ( address == nullptr )
      throw new std::exception( "Couldn't bind" );

    mConnectionInfo.getFrom( mSocket, address, false );
  }

  void TCPSocket::listen()
  {
    if ( mState != State_Disconnected )
      throw new std::exception( "Cannot listen, socket is not disconnected" );

    if ( WSAEventSelect( mSocket, mNetworkEvent, FD_ACCEPT | FD_CLOSE ) == SOCKET_ERROR )
      throw new std::exception( "Couldn't select socket events" );

    if ( ::listen( mSocket, SOMAXCONN ) == SOCKET_ERROR )
      throw new std::exception( "Couldn't listen" );

    mState = State_Listening;
  }

  void TCPSocket::connect( const wstring& host, const wstring& service,
  Protocol protocol )
  {
    if ( mState != State_Disconnected )
      throw new std::exception( "Cannot connect, socket is not disconnected" );

    mState = State_Connecting;

    ADDRINFOW resolve;
    PADDRINFOW address = NULL;
    PADDRINFOW resolved = NULL;

    memset( &resolve, NULL, sizeof( ADDRINFOW ) );

    resolve.ai_family   = util::protocolToFamily( protocol );
    resolve.ai_socktype = SOCK_STREAM;
    resolve.ai_protocol = IPPROTO_TCP;

    if ( GetAddrInfoW( host.c_str(), service.c_str(), &resolve, &resolved ) )
    {
      mState = State_Disconnected;
      throw new std::exception( "Couldn't resolve" );
    }

    for ( address = resolved; address != nullptr; address = address->ai_next )
    {
      if ( address->ai_socktype != SOCK_STREAM || address->ai_protocol != IPPROTO_TCP )
        continue;
      mSocket = WSASocketW( address->ai_family, address->ai_socktype,
        address->ai_protocol, nullptr, 0, 0 );
      if ( mSocket == INVALID_SOCKET )
        continue;
      if ( !WSAConnect( mSocket, address->ai_addr, (int)address->ai_addrlen,
        nullptr, nullptr, nullptr, nullptr ) )
        break;
      closesocket( mSocket );
    }

    if ( address == nullptr )
    {
      mState = State_Disconnected;
      throw new std::exception( "Couldn't connect" );
    }

    mConnectionInfo.getFrom( mSocket, address, true );

    if ( WSAEventSelect( mSocket, mNetworkEvent, FD_READ | FD_CLOSE ) == SOCKET_ERROR )
      throw new std::exception( "Couldn't select socket events" );

    mState = State_Connected;

    for ( SocketListener* listener : mListeners )
      listener->connectCallback( this );
  }

  uint32_t TCPSocket::write( const void* buffer, const uint32_t length )
  {
    if ( mSocket == INVALID_SOCKET )
      throw new std::exception( "Cannot write, socket is invalid" );
    int written = send( mSocket, (const char*)buffer, length, 0 );
    if ( written == SOCKET_ERROR )
      throw new std::exception( "Socket write failed" );
    return written;
  }

  uint32_t TCPSocket::read( void* buffer, uint32_t length )
  {
    if ( mSocket == INVALID_SOCKET )
      throw new std::exception( "Cannot read, socket is invalid" );
    int _read = recv( mSocket, (char*)buffer, length, 0 );
    if ( _read == SOCKET_ERROR )
      throw new std::exception( "Socket read failed" );
    return _read;
  }

  void TCPSocket::process()
  {
    if ( mSocket == INVALID_SOCKET )
      return;
    WSANETWORKEVENTS events;
    if ( WSAEnumNetworkEvents( mSocket, mNetworkEvent, &events ) == SOCKET_ERROR )
      throw new std::exception( "Socket network event processing error" );
    if ( events.lNetworkEvents & FD_READ )
    {
      if ( events.iErrorCode[FD_READ_BIT] != 0 ) {
        mErrors.readEventError = events.iErrorCode[FD_READ_BIT];
        mState = State_Disconnecting;
        mCloseReason = Close_Error;
        close();
        return;
      }
      for ( SocketListener* listener : mListeners )
        listener->readCallback( this );
    }
    if ( events.lNetworkEvents & FD_CLOSE )
    {
      if ( events.iErrorCode[FD_CLOSE_BIT] != 0 ) {
        mErrors.closeEventError = events.iErrorCode[FD_CLOSE_BIT];
        mState = State_Disconnecting;
        mCloseReason = Close_Error;
      } else
        mCloseReason = Close_Graceful;
      close();
    }
  }

  void TCPSocket::closeRequest()
  {
    if ( mState == State_Disconnected )
      return;
    if ( mSocket != INVALID_SOCKET )
      ::shutdown( mSocket, SD_SEND );
    mState = State_Disconnecting;
  }

  void TCPSocket::close()
  {
    if ( mState == State_Disconnected )
      return;
    if ( mSocket != INVALID_SOCKET )
    {
      ::shutdown( mSocket, SD_BOTH );
      closesocket( mSocket );
      mSocket = INVALID_SOCKET;
    }
    mState = State_Disconnected;
    for ( SocketListener* listener : mListeners )
      listener->closeCallback( this );
  }

  TCPSocket::~TCPSocket()
  {
    if ( mNetworkEvent != WSA_INVALID_EVENT )
      WSACloseEvent( mNetworkEvent );
  }

}