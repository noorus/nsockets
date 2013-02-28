#pragma once
#include "nsockets.h"

namespace nsockets {

  namespace util {
    extern inline Protocol familyToProtocol( int family );
    extern inline int protocolToFamily( Protocol protocol );
  }

}