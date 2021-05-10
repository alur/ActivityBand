// Includes Windows.h, with some options.
#pragma once

#if defined(_WINDOWS_) || defined(_WINDOWS_H)
#  error Do not include Windows.h before this header
#endif

// Window target versions
#define _WIN32_WINNT    _WIN32_WINNT_WIN10
#define WINVER          _WIN32_WINNT_WIN10
#define _WIN32_IE       _WIN32_IE_IE110
#define NTDDI_VERSION   NTDDI_WIN10

// Window header options
#define NOCOMM
#define NOCRYPT
#define NOMCX
#define NOMINMAX
#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>