#include <boost/tokenizer.hpp>
#include "nsockets.h"

using std::vector;
using boost::char_separator;
using boost::tokenizer;

using namespace nsockets;

HANDLE stopEvent;

void tokenize( const string& str, string delims, vector<string>& result )
{
  char_separator<char> separator( delims.c_str() );
  tokenizer<char_separator<char>> tokenizer( str, separator );
  for ( string token : tokenizer )
    result.push_back( token );
}

class NaiveIRCClient: public SocketListener {
protected:
  EventTCPSocket socket;
  string buffer;
public:
  NaiveIRCClient( const wstring& host, const wstring& port )
  {
    socket.addListener( this );
    socket.connect( host, port );
  }
  void run( HANDLE event )
  {
    writeLine( "NICK nsockets" );
    writeLine( "USER nsockets 0 * :nsockets" );
    socket.loop( event, 1000, 5000 );
  }
  void writeLine( string line )
  {
    line.append( "\r\n" );
    socket.write( (const void*)&line[0], line.length() );
  }
  void handleLine( const string& line )
  {
    vector<string> tokens;
    string freeline;
    tokenize( line, " ", tokens );
    if ( tokens.empty() )
      return;
    size_t colon = line.find( ':', tokens[0].length() );
    if ( colon != string::npos && colon < line.length() )
      freeline = line.substr( colon + 1, line.length() - colon - 1 );
    if ( !tokens[0].compare( "PING" ) ) {
      writeLine( "PONG :" + freeline );
      printf_s( "ping? pong!\r\n" );
    } else if ( !tokens[0].compare( "ERROR" ) ) {
      printf_s( "error: %s\r\n", freeline.c_str() );
    } else if ( !tokens[0].compare( "NOTICE" ) ) {
      if ( !tokens[1].compare( "AUTH" ) )
        printf_s( "%s\r\n", freeline.c_str() );
    } else if ( tokens[0][0] == ':' ) {
      tokens[0].erase( 0, 1 );
      printf_s( "%s\r\n", line.c_str() );
    }
  }
  virtual bool idleCallback( Socket* _socket )
  {
    wprintf_s( L"idling...\r\n" );
    return true;
  }
  virtual bool acceptCallback( Socket* _socket )
  {
    return false;
  }
  virtual bool connectCallback( Socket* _socket )
  {
    wprintf_s( L"connected - %s:%s -> %s:%s\r\n",
      _socket->getConnectionInfo().getLocalAddress().c_str(),
      _socket->getConnectionInfo().getLocalService().c_str(),
      _socket->getConnectionInfo().getRemoteAddress().c_str(),
      _socket->getConnectionInfo().getRemoteService().c_str()
      );
    return true;
  }
  virtual bool readCallback( Socket* _socket )
  {
    uint8_t current[2048];
    uint32_t read = socket.read( &current, 2048 );
    for ( uint32_t i = 0; i < read; i++ )
    {
      if ( current[i] == 0x0A ) {
        if ( !buffer.empty() )
        {
          handleLine( buffer );
          buffer.clear();
        }
      } else if ( current[i] > 0x1F ) {
        buffer.append( 1, (char)current[i] );
        if ( buffer.length() > 32678 )
          throw new std::exception( "Input buffer exceeded 32kB" );
      }
    }
    return true;
  }
  virtual bool closeCallback( Socket* _socket )
  {
    wprintf_s( L"closed.\r\n" );
    return true;
  }
};

BOOL WINAPI consoleHandler( DWORD ctrl )
{
  if ( ctrl == CTRL_C_EVENT || ctrl == CTRL_CLOSE_EVENT )
  {
    SetEvent( stopEvent );
    return TRUE;
  }
  return FALSE;
}

int wmain( int argc, wchar_t* argv[], wchar_t* envp[] )
{
  try
  {
    nsockets::initialize();
    stopEvent = CreateEventW( 0, FALSE, FALSE, 0 );
    SetConsoleCtrlHandler( consoleHandler, TRUE );
    NaiveIRCClient ircClient(
      L"dreamhack.se.quakenet.org", L"6667" );
    ircClient.run( stopEvent );
    CloseHandle( stopEvent );
  }
  catch ( std::exception& e )
  {
    printf_s( "Exception: %s\r\n", e.what() );
  }
  catch ( ... )
  {
    printf_s( "Unknown exception\r\n" );
  }
  nsockets::shutdown();
  return EXIT_SUCCESS;
}