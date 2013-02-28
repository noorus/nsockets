#pragma once

#include "targetver.h"

// Exclude GDI, CryptoAPI, ShellAPI, Winsock 1
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_

// Platform Headers
#include <windows.h>          // Windows

// CRT Headers
#include <wchar.h>