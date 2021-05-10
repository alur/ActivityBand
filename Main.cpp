#include "ActivityBand.hpp"
#include "ClassFactory.hpp"

#include "Windows.h"
#include <ShlObj.h>

#pragma comment (lib, "advapi32.lib")
#pragma comment (lib, "d2d1.lib")
#pragma comment (lib, "ole32.lib")
#pragma comment (lib, "pdh.lib")
#pragma comment (lib, "shell32.lib")
#pragma comment (lib, "user32.lib")

// The handle to this DLL.
HMODULE module;

// The number of in-use objects.
long objectCounter;

// The CLSID of ActivityBand, {890fd4d5-1be6-4058-9c30-c23ee7b06dc2}.
extern const CLSID CLSID_ActivityBand = { 0x890fd4d5, 0x1be6, 0x4058, {0x9c, 0x30, 0xc2, 0x3e, 0xe7, 0xb0, 0x6d, 0xc2} };

/// <summary>
/// The main entry point for this DLL.
/// </summary>
/// <param name="module">A handle to the DLL module.</param>
/// <param name="reasonForCall">The reason code that indicates why the DLL entry-point function is being called.</param>
/// <returns>TRUE on success, FALSE on failure.</returns>
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reasonForCall, LPVOID /* reserved */) {
  if (reasonForCall == DLL_PROCESS_ATTACH) {
    // while (!IsDebuggerPresent()) {
    //  Sleep(100);
    // }
    DisableThreadLibraryCalls(module);
    ::module = hModule;
    ::objectCounter = 0;

    ActivityBand::StaticInit();
  } else if (reasonForCall == DLL_PROCESS_DETACH) {
    ActivityBand::StaticTearDown();
  }
  return TRUE;
}

/// <summary>
/// Determines whether the DLL that implements this function is in use.
/// </summary>
__control_entrypoint(DllExport) STDAPI DllCanUnloadNow() {
  return ::objectCounter == 0 ? S_OK : S_FALSE;
}

/// <summary>
/// Retrieves the class object from a DLL object handler or object application.
/// </summary>
_Check_return_ STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID *ppv) {
  if (rclsid != CLSID_ActivityBand) return CLASS_E_CLASSNOTAVAILABLE;

  ClassFactory *factory = new ClassFactory();
  if (factory == nullptr) return E_OUTOFMEMORY;

  HRESULT hr = factory->QueryInterface(riid, ppv);
  factory->Release();
  return hr;
}