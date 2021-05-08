// Handles registering the DLL with explorer.
#pragma once

#include "Windows.h"

// regsvr32 entry points.
STDAPI DllRegisterServer();
STDAPI DllUnregisterServer();

// Elevation stuff
bool IsElevated();
HRESULT RunElevated(LPCWSTR);
void CALLBACK DllElevatedEntry(HWND, HINSTANCE, LPSTR, int);

// Registration/Unregistration
HRESULT GetKeyPath(LPWSTR path, UINT cchPath);
HRESULT DeleteRegKeys();
HRESULT RegisterDLL();
HRESULT RegisterComCat();
HRESULT UnRegisterComCat();