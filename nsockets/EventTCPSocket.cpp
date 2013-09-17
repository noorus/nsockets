#include "nsockets.h"
#include "nsocketsUtil.h"

namespace nsockets {

  EventTCPSocket::EventTCPSocket( bool overlapped ):
  TCPSocket( overlapped ), mNetworkEvent( WSA_INVALID_EVENT )
  {
    mNetworkEvent = WSACreateEvent();
    if ( mNetworkEvent == WSA_INVALID_EVENT )
      EXCEPT_WSA( L"Couldn't create network event" );
  }

  void EventTCPSocket::listen()
  {
    if ( mState != State_Disconnected )
      EXCEPT( L"Cannot listen, socket is not disconnected" );

    if ( WSAEventSelect( mSocket, mNetworkEvent, FD_ACCEPT | FD_CLOSE ) == SOCKET_ERROR )
      EXCEPT_WSA( L"Couldn't select socket events" );

    if ( ::listen( mSocket, SOMAXCONN ) == SOCKET_ERROR )
      EXCEPT_WSA( L"Couldn't listen" );

    mState = State_Listening;
  }

  void EventTCPSocket::connect( const wstring& host,
  const wstring& service, Protocol protocol )
  {
    if ( mState != State_Disconnected )
      EXCEPT( L"Cannot connect, socket is not disconnected" );

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
      EXCEPT_WSA( L"Couldn't resolve" );
    }

    for ( address = resolved; address != nullptr; address = address->ai_next )
    {
      if ( address->ai_socktype != SOCK_STREAM || address->ai_protocol != IPPROTO_TCP )
        continue;

      mSocket = WSASocketW( address->ai_family, address->ai_socktype,
        address->ai_protocol, nullptr, 0,
        mOverlapped ? WSA_FLAG_OVERLAPPED : 0 );

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
      EXCEPT_WSA( L"Couldn't connect" );
    }

    mConnectionInfo.getFrom( mSocket, address, true );

    if ( WSAEventSelect( mSocket, mNetworkEvent, FD_READ | FD_CLOSE ) == SOCKET_ERROR )
      EXCEPT_WSA( L"Couldn't select socket events" );

    mState = State_Connected;

    for ( SocketListener* listener : mListeners )
      listener->connectCallback( this );
  }

  void EventTCPSocket::process()
  {
    if ( mSocket == INVALID_SOCKET )
      return;

    WSANETWORKEVENTS events;

    if ( WSAEnumNetworkEvents( mSocket, mNetworkEvent, &events ) == SOCKET_ERROR )
      EXCEPT_WSA( L"Socket network event processing error" );

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

  EventTCPSocket::~EventTCPSocket()
  {
    if ( mNetworkEvent != WSA_INVALID_EVENT )
      WSACloseEvent( mNetworkEvent );
  }

}