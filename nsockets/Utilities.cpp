#include "stdafx.h"
#include "nsockets.h"

namespace nsockets {

  using namespace util;

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