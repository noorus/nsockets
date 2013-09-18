#include "nsockets.h"
#include "nsocketsUtil.h"

namespace nsockets {

  ConnectionInfo::ConnectionInfo(): mProtocol( Protocol_Unknown )
  {
  }

  void ConnectionInfo::reset()
  {
    mRemoteAddress.clear();
    mRemoteService.clear();
    mLocalAddress.clear();
    mLocalService.clear();
    mProtocol = Protocol_Unknown;
  }

  void ConnectionInfo::update( SOCKET socket, bool remote )
  {
    SOCKADDR_STORAGE socketAddress;
    int length = sizeof( socketAddress );
    memset( &socketAddress, NULL, length );

    WCHAR tmpAddress[NI_MAXHOST];
    WCHAR tmpService[NI_MAXSERV];

    if ( remote )
    {
      if ( getpeername( socket, (LPSOCKADDR)&socketAddress, &length ) == SOCKET_ERROR )
        EXCEPT_WSA( L"Couldn't fetch peer name" );

      if ( GetNameInfoW( (LPSOCKADDR)&socketAddress, length, tmpAddress,
        NI_MAXHOST, tmpService, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV ) )
        EXCEPT_WSA( L"Couldn't fetch peer name info" );

      mRemoteAddress = tmpAddress;
      mRemoteService = tmpService;

      mProtocol = util::familyToProtocol( socketAddress.ss_family );
    }
    else
    {
      if ( getsockname( socket, (LPSOCKADDR)&socketAddress, &length ) == SOCKET_ERROR )
        EXCEPT_WSA( L"Couldn't fetch socket name" );

      if ( GetNameInfoW( (LPSOCKADDR)&socketAddress, length, tmpAddress,
        NI_MAXHOST, tmpService, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV ) )
        EXCEPT_WSA( L"Couldn't fetch socket name info" );

      mLocalAddress = tmpAddress;
      mLocalService = tmpService;

      mProtocol = util::familyToProtocol( socketAddress.ss_family );
    }
  }

  const wstring& ConnectionInfo::getRemoteAddress() const throw()
  {
    return mRemoteAddress;
  }

  const wstring& ConnectionInfo::getRemoteService() const throw()
  {
    return mRemoteService;
  }

  const wstring& ConnectionInfo::getLocalAddress() const throw()
  {
    return mLocalAddress;
  }

  const wstring& ConnectionInfo::getLocalService() const throw()
  {
    return mLocalService;
  }

  const Protocol ConnectionInfo::getProtocol() const throw()
  {
    return mProtocol;
  }

}