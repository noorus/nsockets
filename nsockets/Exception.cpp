#include "nsockets.h"
#include "nsocketsUtil.h"

namespace nsockets {

  Exception::Exception()
  {
    // Stub
  }

  Exception::Exception( const wstring& description, Type type ):
  mDescription( description ), mType( type )
  {
    // Stub
  }

  Exception::Exception( const wstring& description, const wstring& source, Type type ):
  mDescription( description ), mSource( source ), mType( type )
  {
    // Stub
  }

  void Exception::handleAdditional()
  {
    switch ( mType )
    {
      case Winsock:
        WinAPIError error;
        LPWSTR message = nullptr;
        error.code = WSAGetLastError();
        FormatMessageW(
          FORMAT_MESSAGE_ALLOCATE_BUFFER |
          FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
          NULL, error.code, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
          (LPWSTR)&message, 0, NULL );
        error.description = message;
        LocalFree( message );
        mAdditional = error;
      break;
    }
  }

  const wstring& Exception::getFullDescription() const
  {
    if ( mFullDescription.empty() )
    {
      wstringstream stream;

      stream << mDescription;

      if ( !mSource.empty() )
        stream << L"\r\nIn function " << mSource;

      switch ( mType )
      {
        case Winsock:
          const WinAPIError& error = boost::get<WinAPIError>( mAdditional );
          stream << L"\r\nWSA error code " << std::hex << error.code << L":\r\n" << error.description;
        break;
      }

      mFullDescription = stream.str();
    }
    return mFullDescription;
  }

  const char* Exception::what() const
  {
    return util::wideToUtf8( getFullDescription() ).c_str();
  }

}