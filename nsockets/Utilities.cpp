#include "nsockets.h"
#include "nsocketsUtil.h"

namespace nsockets {

  WSADATA g_wsaData = { 0 };

  void initialize()
  {
    WORD version = MAKEWORD( 2, 2 );
    if ( WSAStartup( version, &g_wsaData ) )
      throw new std::exception( "Winsock initialization failed" );
    if ( g_wsaData.wVersion != version )
      throw new std::exception( "Required Winsock version not available" );
  }

  void shutdown()
  {
    if ( g_wsaData.wVersion > 0 )
      WSACleanup();
  }

  namespace util {

  inline Protocol familyToProtocol( int family )
  {
    switch ( family )
    {
      case PF_UNSPEC: return Protocol_Any; break;
      case PF_INET:   return Protocol_IPv4; break;
      case PF_INET6:  return Protocol_IPv6; break;
    }
    return Protocol_Unknown;
  }

  inline int protocolToFamily( Protocol protocol )
  {
    switch ( protocol )
    {
    case Protocol_Any:  return PF_UNSPEC; break;
    case Protocol_IPv4: return PF_INET; break;
    case Protocol_IPv6: return PF_INET6; break;
    }
    return PF_UNSPEC;
  }

  }

}