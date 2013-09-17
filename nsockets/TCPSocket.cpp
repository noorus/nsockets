#include "nsockets.h"
#include "nsocketsUtil.h"

namespace nsockets {

  TCPSocket::TCPSocket( bool overlapped ): Socket(),
  mState( State_Disconnected ), mOverlapped( overlapped )
  {
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
      EXCEPT( L"Cannot bind, socket is not disconnected" );

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

    mConnectionInfo.getFrom( mSocket, address, false );
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
  }

}