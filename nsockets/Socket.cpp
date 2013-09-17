#include "nsockets.h"
#include "nsocketsUtil.h"

namespace nsockets {

  void ConnectionInfo::getFrom( SOCKET socket, PADDRINFOW paddress, bool remote )
  {
    int length = sizeof( socketAddress );

    int ret;
    if ( remote )
      ret = getpeername( socket, (LPSOCKADDR)&socketAddress, &length );
    else
      ret = getsockname( socket, (LPSOCKADDR)&socketAddress, &length );

    if ( ret == SOCKET_ERROR )
      EXCEPT_WSA( L"Couldn't fetch socket/peer name" );

    ret = GetNameInfoW( (LPSOCKADDR)&socketAddress, length, hostAddress,
      NI_MAXHOST, hostService, NI_MAXSERV, NI_NUMERICHOST );
    if ( ret )
      EXCEPT_WSA( L"Couldn't fetch socket name info" );

    memcpy_s( &addressInfo, sizeof( addressInfo ), paddress, sizeof( addressInfo ) );
  }

  Socket::Socket(): mSocket( INVALID_SOCKET )
  {
    memset( &mConnectionInfo, 0, sizeof( ConnectionInfo ) );
  }

  void Socket::addListener( SocketListener* listener )
  {
    mListeners.push_back( listener );
  }

  void Socket::removeListener( SocketListener* listener )
  {
    mListeners.remove( listener );
  }

  SOCKET Socket::getRawSocket()
  {
    return mSocket;
  }
  
  const Protocol Socket::getProtocol()
  {
    return util::familyToProtocol( mConnectionInfo.addressInfo.ai_family );
  }

  const ConnectionInfo& Socket::getConnectionInfo()
  {
    return mConnectionInfo;
  }

}