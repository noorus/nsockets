#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <wchar.h>
#include <exception>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <boost/tokenizer.hpp>

#include "nsockets.h"

using std::string;
using std::vector;
using boost::char_separator;
using boost::tokenizer;

using namespace nsockets;

void tokenize( const string& str, string delims, vector<string>& result )
{
  char_separator<char> separator( delims.c_str() );
  tokenizer<char_separator<char>> tokenizer( str, separator );
  for ( string token : tokenizer )
    result.push_back( token );
}

class NaiveIRCClient: public SocketListener {
protected:
  TCPSocket socket;
  string buffer;
public:
  NaiveIRCClient( const wstring& host, const wstring& port )
  {
    socket.addListener( this );
    socket.connect( host, port );
  }
  void run()
  {
    writeLine( "NICK nsockets" );
    writeLine( "USER nsockets 0 * :nsockets" );
    while ( socket.getState() != TCPSocket::State_Disconnected )
      socket.process();
  }
  void writeLine( string line )
  {
    line.append( "\r\n" );
    printf_s( "-> %s", line.c_str() );
    socket.write( (const void*)&line[0], line.length() );
  }
  void handleLine( const string& line )
  {
    // printf_s( "<- %s\r\n", line.c_str() );
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
  virtual void readCallback( Socket* _notused )
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
          throw new std::exception( "Something's fishy here; input buffer exceeded 32kB" );
      }
    }
  }
  virtual void closeCallback( nsockets::Socket* _notused )
  {
    wprintf_s( L"closed!\r\n" );
  }
};

int wmain( int argc, wchar_t* argv[], wchar_t* envp[] )
{
  WSADATA wsaData;
  WSAStartup( MAKEWORD(2,2), &wsaData );
  try
  {
    NaiveIRCClient ircClient(
      L"dreamhack.se.quakenet.org", L"6667" );
    ircClient.run();
  }
  catch ( std::exception& e )
  {
    printf_s( "Exception: %s\r\n", e.what() );
  }
  catch ( ... )
  {
    printf_s( "Unknown exception\r\n" );
  }
  WSACleanup();
  return EXIT_SUCCESS;
}