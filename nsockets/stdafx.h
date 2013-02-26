#pragma once

#include "targetver.h"

// Exclude GDI, CryptoAPI, ShellAPI, Winsock 1
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_

// Platform Headers
#include <windows.h>          // Windows
#include <winsock2.h>         // WinSock 2                (ws2_32.lib)
#include <ws2tcpip.h>         // WinSock 2 TCP/IP         (ws2_32.lib)
#include <iphlpapi.h>         // IP Helper API            (iphlpapi.lib)

// CRT Headers
#include <wchar.h>

// STL Headers
#include <exception>
#include <memory>
#include <string>
#include <vector>
#include <list>
#include <map>