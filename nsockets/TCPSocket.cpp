#include "nsockets.h"
#include "nsocketsUtil.h"

namespace nsockets {

  TCPSocket::TCPSocket( bool overlapped ): Socket(),
  mState( State_Closed ), mOverlapped( overlapped ), mBound( false )
  {
    memset( &mErrors, 0, sizeof( Errors ) );
  }

  const TCPSocket::State& TCPSocket::getState()
  {
    return mState;
  }

  void TCPSocket::disableNagle( bool disable )
  {
    if ( mSocket == INVALID_SOCKET )
      EXCEPT( L"Cannot modify socket options, socket does not exist" );

    BOOL value = disable ? TRUE : FALSE;

    if ( setsockopt( mSocket, IPPROTO_TCP, TCP_NODELAY,
      reinterpret_cast<const char*>(&value), sizeof( value ) ) == SOCKET_ERROR )
      EXCEPT_WSA( L"Couldn't disable Nagle on socket" );
  }

  void TCPSocket::setExclusiveAddr( bool exclusive )
  {
    if ( mSocket == INVALID_SOCKET )
      EXCEPT( L"Cannot modify socket options, socket does not exist" );

    if ( mBound )
      EXCEPT( L"Cannot set local address exclusivity on a bound socket" );

    BOOL value = exclusive ? 1 : 0;

    if ( setsockopt( mSocket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE,
      reinterpret_cast<const char*>(&value), sizeof( value ) ) == SOCKET_ERROR )
      EXCEPT_WSA( L"Couldn't set local address exclusivity on socket" );
  }

  void TCPSocket::setKeepAlive( bool keepalive, uint32_t milliseconds )
  {
    if ( mSocket == INVALID_SOCKET )
      EXCEPT( L"Cannot modify socket options, socket does not exist" );

    tcp_keepalive value;
    value.onoff             = keepalive ? 1 : 0;
    value.keepalivetime     = milliseconds;
    value.keepaliveinterval = milliseconds;

    DWORD returned;
    if ( !WSAIoctl( mSocket, SIO_KEEPALIVE_VALS, &value, sizeof( value ), nullptr, 0, &returned, nullptr, nullptr ) )
      EXCEPT_WSA( L"Couldn't set keepalive values on socket" );
  }

  void TCPSocket::bind( const wstring& host, const wstring& service,
  Protocol protocol )
  {
    if ( mState != State_Closed )
      EXCEPT( L"Cannot bind, socket is not closed" );

    ADDRINFOW resolve;
    PADDRINFOW address = nullptr;
    PADDRINFOW resolved = nullptr;

    memset( &resolve, 0, sizeof( ADDRINFOW ) );

    resolve.ai_family   = util::protocolToFamily( protocol );
    resolve.ai_socktype = SOCK_STREAM;
    resolve.ai_protocol = IPPROTO_TCP;

    if ( GetAddrInfoW( host.c_str(), service.c_str(), &resolve, &resolved ) )
      EXCEPT_WSA( L"Couldn't resolve" );

    for ( address = resolved; address != nullptr; address = address->ai_next )
    {
      if ( address->ai_socktype != SOCK_STREAM || address->ai_protocol != IPPROTO_TCP )
        continue;

      mSocket = WSASocketW(
        address->ai_family, address->ai_socktype, address->ai_protocol,
        nullptr, 0, mOverlapped ? WSA_FLAG_OVERLAPPED : 0 );

      if ( mSocket == INVALID_SOCKET )
        continue;

      if ( ::bind( mSocket, address->ai_addr, (int)address->ai_addrlen ) != SOCKET_ERROR )
        break;

      closesocket( mSocket );
    }

    if ( address == nullptr )
      EXCEPT_WSA( L"Couldn't bind" );

    mBound = true;

    mConnectionInfo.update( mSocket, false );
  }

  uint32_t TCPSocket::write( const void* buffer, const uint32_t length )
  {
    if ( mSocket == INVALID_SOCKET )
      EXCEPT( L"Cannot write, socket does not exist" );
    int written = send( mSocket, (const char*)buffer, length, 0 );
    if ( written == SOCKET_ERROR )
      EXCEPT_WSA( L"Socket write failed" );
    return written;
  }

  uint32_t TCPSocket::read( void* buffer, uint32_t length )
  {
    if ( mSocket == INVALID_SOCKET )
      EXCEPT( L"Cannot read, socket does not exist" );
    int _read = recv( mSocket, (char*)buffer, length, 0 );
    if ( _read == SOCKET_ERROR )
      EXCEPT_WSA( L"Socket read failed" );
    return _read;
  }

  void TCPSocket::closeRequest()
  {
    if ( mState == State_Closed )
      return;

    if ( mSocket != INVALID_SOCKET )
      ::shutdown( mSocket, SD_SEND );

    mState = State_Closing;
  }

  void TCPSocket::close()
  {
    if ( mState == State_Closed )
      return;

    if ( mSocket != INVALID_SOCKET )
    {
      ::shutdown( mSocket, SD_BOTH );
      closesocket( mSocket );
      mSocket = INVALID_SOCKET;
    }

    mState = State_Closed;

    for ( SocketListener* listener : mListeners )
      if ( listener->closeCallback( this ) )
        break;
  }

  TCPSocket::~TCPSocket()
  {
  }

}