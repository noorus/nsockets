#pragma once
#include "nsockets.h"

namespace nsockets {

  namespace util {
    extern inline Protocol familyToProtocol( uint16_t family );
    extern inline uint16_t protocolToFamily( Protocol protocol );
    extern inline wstring utf8ToWide( const string& in ) throw();
    extern inline string wideToUtf8( const wstring& in ) throw();
  }

#if defined(EXCEPT) || defined(EXCEPT_WINSOCK) || defined(EXCEPT_WINAPI)
# error EXCEPT* maro already defined!
#else
# define EXCEPT(description) {throw nsockets::Exception(description,__FUNCTIONW__,nsockets::Exception::Generic);}
# define EXCEPT_WSA(description) {throw nsockets::Exception(description,__FUNCTIONW__,nsockets::Exception::Winsock);}
# define EXCEPT_WINAPI(description) {throw nsockets::Exception(description,__FUNCTIONW__,nsockets::Exception::WinAPI);}
#endif

}