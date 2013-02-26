#include "stdafx.h"
#include "nsockets.h"

namespace nsockets {

  TCPSocket::TCPSocket(): Socket(), mNetworkEvent( WSA_INVALID_EVENT )
  {
    mNetworkEvent = WSACreateEvent();
    if ( mNetworkEvent == WSA_INVALID_EVENT )
      throw new std::exception( "Couldn't create network event" );
  }

  void TCPSocket::bind( const wstring& host, const wstring& service,
  Protocol protocol )
  {
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

      if ( ::bind( mSocket, address->ai_addr, address->ai_addrlen ) != SOCKET_ERROR )
        break;

      closesocket( mSocket );
    }

    if ( address == nullptr )
      throw new std::exception( "Couldn't bind" );

    mConnectionInfo.getFrom( mSocket, address, false );
  }

  void TCPSocket::listen()
  {
    if ( WSAEventSelect( mSocket, mNetworkEvent, FD_ACCEPT | FD_CLOSE ) == SOCKET_ERROR )
      throw new std::exception( "Couldn't select socket events" );

    if ( ::listen( mSocket, SOMAXCONN ) == SOCKET_ERROR )
      throw new std::exception( "Couldn't listen" );
  }

  void TCPSocket::connect( const wstring& host, const wstring& service,
  Protocol protocol )
  {
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

      if ( !WSAConnect( mSocket, address->ai_addr, address->ai_addrlen, nullptr, nullptr, nullptr, nullptr ) )
        break;

      closesocket( mSocket );
    }

    if ( address == nullptr )
      throw new std::exception( "Couldn't connect" );

    mConnectionInfo.getFrom( mSocket, address, true );

    if ( WSAEventSelect( mSocket, mNetworkEvent, FD_READ | FD_CLOSE ) == SOCKET_ERROR )
      throw new std::exception( "Couldn't select socket events" );
  }

  void TCPSocket::closeRequest()
  {
    shutdown( mSocket, SD_SEND );
  }

  void TCPSocket::close()
  {
    shutdown( mSocket, SD_BOTH );
    closesocket( mSocket );
  }

  TCPSocket::~TCPSocket()
  {
    if ( mNetworkEvent != WSA_INVALID_EVENT )
      WSACloseEvent( mNetworkEvent );
  }

}