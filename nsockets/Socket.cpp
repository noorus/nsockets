#include "nsockets.h"
#include "nsocketsUtil.h"

namespace nsockets {

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

  SOCKET Socket::getRawSocket() const throw()
  {
    return mSocket;
  }
  
  const Protocol Socket::getProtocol() const throw()
  {
    return mConnectionInfo.getProtocol();
  }

  const ConnectionInfo& Socket::getConnectionInfo()
  {
    return mConnectionInfo;
  }

}