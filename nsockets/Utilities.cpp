#include "nsockets.h"
#include "nsocketsUtil.h"

namespace nsockets {

  WSADATA g_wsaData = { 0 };

  void initialize()
  {
    WORD version = MAKEWORD( 2, 2 );
    if ( WSAStartup( version, &g_wsaData ) )
      EXCEPT_WSA( L"Winsock initialization failed" );
    if ( g_wsaData.wVersion != version )
      EXCEPT_WSA( L"Required Winsock version not available" );
  }

  void shutdown()
  {
    if ( g_wsaData.wVersion > 0 )
      WSACleanup();
  }

  namespace util {

    inline Protocol familyToProtocol( uint16_t family )
    {
      if ( family == PF_UNSPEC || family == AF_UNSPEC )
        return Protocol_Any;
      else if ( family == PF_INET || family == AF_INET )
        return Protocol_IPv4;
      else if ( family == PF_INET6 || family == AF_INET6 )
        return Protocol_IPv6;
      else
        return Protocol_Unknown;
    }

    inline uint16_t protocolToFamily( Protocol protocol )
    {
      switch ( protocol )
      {
        case Protocol_Any:  return PF_UNSPEC; break;
        case Protocol_IPv4: return PF_INET; break;
        case Protocol_IPv6: return PF_INET6; break;
      }
      return PF_UNSPEC;
    }

    inline wstring utf8ToWide( const string& in ) throw()
    {
      int length = MultiByteToWideChar( CP_UTF8, 0, in.c_str(), -1, nullptr, 0 );
      if ( length == 0 )
        return wstring();
      vector<wchar_t> conversion( length );
      MultiByteToWideChar( CP_UTF8, 0, in.c_str(), -1, &conversion[0], length );
      return wstring( &conversion[0] );
    }

    inline string wideToUtf8( const wstring& in ) throw()
    {
      int length = WideCharToMultiByte( CP_UTF8, 0, in.c_str(), -1, nullptr, 0, 0, FALSE );
      if ( length == 0 )
        return string();
      vector<char> conversion( length );
      WideCharToMultiByte( CP_UTF8, 0, in.c_str(), -1, &conversion[0], length, 0, FALSE );
      return string( &conversion[0] );
    }

  }

}